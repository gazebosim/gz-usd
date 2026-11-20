/*
 * Copyright (C) 2022 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "USDLinks.hh"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#pragma push_macro ("__DEPRECATED")
#undef __DEPRECATED
#include <pxr/usd/kind/registry.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/gprim.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/subset.h>
#include "pxr/usd/usdPhysics/collisionAPI.h"
#include "pxr/usd/usdPhysics/massAPI.h"
#include <pxr/usd/usdPhysics/rigidBodyAPI.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#pragma pop_macro ("__DEPRECATED")

#include <gz/common/ColladaExporter.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Material.hh>
#include <gz/common/Mesh.hh>
#include <gz/common/SubMesh.hh>
#include <gz/common/Util.hh>

#include <gz/math/Inertial.hh>

#include "sdf/Box.hh"
#include "sdf/Collision.hh"
#include "sdf/Cone.hh"
#include "sdf/Cylinder.hh"
#include "sdf/Geometry.hh"
#include "sdf/Link.hh"
#include "sdf/Mesh.hh"
#include "sdf/Sphere.hh"
#include "sdf/Visual.hh"

#include "polygon_helper.hh"

#include "gz/usd/usd_parser/USDTransforms.hh"
#include "gz/usd/UsdError.hh"
#include "../Conversions.hh"

namespace gz
{
// Inline bracket to help doxygen filtering.
inline namespace GZ_USD_VERSION_NAMESPACE {
//
namespace usd
{
//////////////////////////////////////////////////
void GetInertial(
  const pxr::UsdPrim &_prim,
  math::Inertiald &_inertial)
{
  float mass;
  pxr::GfVec3f centerOfMass;
  pxr::GfVec3f diagonalInertia;
  pxr::GfQuatf principalAxes;

  math::MassMatrix3d massMatrix;

  if (_prim.HasAPI<pxr::UsdPhysicsRigidBodyAPI>())
  {
    if (pxr::UsdAttribute massAttribute =
      _prim.GetAttribute(pxr::TfToken("physics:mass")))
    {
      massAttribute.Get(&mass);

      if (pxr::UsdAttribute centerOfMassAttribute =
        _prim.GetAttribute(pxr::TfToken("physics:centerOfMass")))
      {
        centerOfMassAttribute.Get(&centerOfMass);
      }

      if (pxr::UsdAttribute diagonalInertiaAttribute =
        _prim.GetAttribute(pxr::TfToken("physics:diagonalInertia")))
      {
        diagonalInertiaAttribute.Get(&diagonalInertia);
      }

      if (pxr::UsdAttribute principalAxesAttribute =
        _prim.GetAttribute(pxr::TfToken("physics:principalAxes")))
      {
        principalAxesAttribute.Get(&principalAxes);
      }

      // Added a diagonal inertia to avoid crash with the physics engine
      if (diagonalInertia == pxr::GfVec3f(0, 0, 0))
      {
        diagonalInertia = pxr::GfVec3f(0.0001, 0.0001, 0.0001);
      }

      // Added a mass to avoid crash with the physics engine
      if (mass < 0.0001)
      {
        mass = 0.1;
      }

      massMatrix.SetMass(mass);

      // TODO(ahcorde) Figure out how to use PrincipalMoments and
      // PrincipalAxesOffset: see
      // https://github.com/gazebosim/sdformat/pull/902#discussion_r840905534
      massMatrix.SetDiagonalMoments(
        math::Vector3d(
          diagonalInertia[0],
          diagonalInertia[1],
          diagonalInertia[2]));

      _inertial.SetPose(math::Pose3d(
          math::Vector3d(
            centerOfMass[0], centerOfMass[1], centerOfMass[2]),
          math::Quaterniond(1.0, 0, 0, 0)));

      _inertial.SetMassMatrix(massMatrix);
    }
  }
  else
  {
    auto parent = _prim.GetParent();
    if (parent)
    {
      GetInertial(parent, _inertial);
    }
  }
}

//////////////////////////////////////////////////
/// \brief Create the directory based on the USD path
/// \param[in] _primPath Reference to the USD prim
/// \return A string with the path
std::string directoryFromUSDPath(const std::string &_primPath)
{
  std::vector<std::string> tokensChild =
    gz::common::split(_primPath, "/");
  std::string directoryMesh;
  if (tokensChild.size() > 1)
  {
    directoryMesh = tokensChild[0];
    for (unsigned int i = 1; i < tokensChild.size() - 1; ++i)
    {
        directoryMesh = gz::common::joinPaths(
          directoryMesh, tokensChild[i+1]);
    }
  }
  return directoryMesh;
}

//////////////////////////////////////////////////
/// \brief Append the raw bytes of a value to a key
/// \param[in,out] _key Key to append to
/// \param[in] _value Value to append
template<typename T>
void appendToKey(std::string &_key, const T &_value)
{
  _key.append(reinterpret_cast<const char *>(&_value), sizeof(T));
}

//////////////////////////////////////////////////
/// \brief Append a string to a key, prefixed by its size
/// \param[in,out] _key Key to append to
/// \param[in] _value String to append
void appendToKey(std::string &_key, const std::string &_value)
{
  appendToKey(_key, _value.size());
  _key.append(_value);
}

//////////////////////////////////////////////////
/// \brief Build a key that identifies all the mesh data written by the
/// Collada exporter (geometry and materials). Two meshes with the same key
/// produce the same Collada file.
/// \param[in] _mesh Mesh to build the key of
/// \return SHA-1 hash of the mesh data
std::string meshKey(const gz::common::Mesh &_mesh)
{
  std::string key;

  appendToKey(key, _mesh.MaterialCount());
  for (unsigned int i = 0; i < _mesh.MaterialCount(); ++i)
  {
    const auto material = _mesh.MaterialByIndex(i);
    if (!material)
    {
      appendToKey(key, false);
      continue;
    }
    appendToKey(key, true);
    appendToKey(key, material->Ambient().AsRGBA());
    appendToKey(key, material->Diffuse().AsRGBA());
    appendToKey(key, material->Specular().AsRGBA());
    appendToKey(key, material->Emissive().AsRGBA());
    appendToKey(key, material->Transparency());
    appendToKey(key, material->Shininess());
    appendToKey(key, material->TextureImage());
  }

  appendToKey(key, _mesh.SubMeshCount());
  for (unsigned int i = 0; i < _mesh.SubMeshCount(); ++i)
  {
    const auto subMesh = _mesh.SubMeshByIndex(i).lock();
    if (!subMesh)
    {
      appendToKey(key, false);
      continue;
    }
    appendToKey(key, true);
    appendToKey(key, subMesh->SubMeshPrimitiveType());

    const auto materialIndex = subMesh->GetMaterialIndex();
    appendToKey(key, materialIndex.has_value());
    if (materialIndex)
      appendToKey(key, materialIndex.value());

    appendToKey(key, subMesh->VertexCount());
    for (unsigned int j = 0; j < subMesh->VertexCount(); ++j)
    {
      const auto &v = subMesh->Vertex(j);
      appendToKey(key, v.X());
      appendToKey(key, v.Y());
      appendToKey(key, v.Z());
    }

    appendToKey(key, subMesh->NormalCount());
    for (unsigned int j = 0; j < subMesh->NormalCount(); ++j)
    {
      const auto &n = subMesh->Normal(j);
      appendToKey(key, n.X());
      appendToKey(key, n.Y());
      appendToKey(key, n.Z());
    }

    const unsigned int texCoordCount = subMesh->TexCoordCountBySet(0);
    appendToKey(key, texCoordCount);
    for (unsigned int j = 0; j < texCoordCount; ++j)
    {
      const auto &t = subMesh->TexCoordBySet(j, 0);
      appendToKey(key, t.X());
      appendToKey(key, t.Y());
    }

    appendToKey(key, subMesh->IndexCount());
    for (unsigned int j = 0; j < subMesh->IndexCount(); ++j)
    {
      appendToKey(key, subMesh->Index(j));
    }
  }

  return gz::common::sha1(key);
}

//////////////////////////////////////////////////
/// \brief Export a mesh as Collada in _directoryMesh. Scenes such as Isaac Sim
/// worlds contain many copies of the same mesh (e.g. hundreds of identical
/// plants), so if a mesh with the same data was already exported, it is not
/// exported again and the path of the existing file is used instead.
/// \param[in] _mesh Mesh to export
/// \param[in] _directoryMesh Directory where the mesh is exported. The file
/// is named after the last element of the directory.
/// \param[in,out] _usdData USD data where the exported meshes are registered
/// \param[out] _file Path of the Collada file to use
/// \return gz::usd::UsdErrors, which is a list of UsdError objects.
/// An empty list means the mesh was exported successfully
gz::usd::UsdErrors exportMesh(const gz::common::Mesh &_mesh,
  const std::string &_directoryMesh,
  USDData &_usdData,
  std::string &_file)
{
  gz::usd::UsdErrors errors;

  const std::string key = meshKey(_mesh);
  _file = _usdData.ExportedMesh(key);
  if (!_file.empty())
    return errors;

  if (!gz::common::createDirectories(_directoryMesh))
  {
    errors.emplace_back(UsdErrorCode::USD_TO_GZ_USD_PARSING_ERROR,
      "Unable to create directory [" + _directoryMesh + "]");
    return errors;
  }

  const std::string fileNoExtension = gz::common::joinPaths(
    _directoryMesh, gz::common::basename(_directoryMesh));
  gz::common::ColladaExporter exporter;
  exporter.Export(&_mesh, fileNoExtension, false);
  _file = fileNoExtension + ".dae";

  if (!gz::common::isFile(_file))
  {
    errors.emplace_back(UsdErrorCode::USD_TO_GZ_USD_PARSING_ERROR,
      "Unable to export mesh [" + _file + "]");
    return errors;
  }

  _usdData.AddExportedMesh(key, _file);
  return errors;
}

//////////////////////////////////////////////////
/// \brief Get the material name of the prim
/// \param[in] _prim Reference to the USD prim
/// \return A string with the material name
std::string ParseMaterialName(const pxr::UsdPrim &_prim)
{
  // Materials
  std::string nameMaterial;

  auto bindMaterial = pxr::UsdShadeMaterialBindingAPI(_prim);
  pxr::UsdRelationship directBindingRel =
    bindMaterial.GetDirectBindingRel();

  pxr::SdfPathVector paths;
  directBindingRel.GetTargets(&paths);
  for (const auto & p : paths)
  {
    std::vector<std::string> tokensMaterial =
      gz::common::split(pxr::TfStringify(p), "/");

    if(tokensMaterial.size() > 0)
    {
      nameMaterial = tokensMaterial[tokensMaterial.size() - 1];
    }
  }
  return nameMaterial;
}

//////////////////////////////////////////////////
/// \brief Parse USD mesh subsets
/// \param[in] _prim Reference to the USD prim
/// \param[in] _link Current link
/// \param[in] _subMesh submesh to add the subsets
/// \param[in] _meshGeom Mesh geom
/// \param[inout] _scale scale mesh
/// \param[in,out] _usdData metadata of the USD file
/// \param[out] _errors Errors found while parsing the mesh subsets
/// \return Number of mesh subsets
int ParseMeshSubGeom(const pxr::UsdPrim &_prim,
  sdf::Link *_link,
  gz::common::SubMesh &_subMesh,
  sdf::Mesh &_meshGeom,
  math::Vector3d &_scale,
  USDData &_usdData,
  gz::usd::UsdErrors &_errors)
{
  int numSubMeshes = 0;

  for (const auto & child : _prim.GetChildren())
  {
    if (child.IsA<pxr::UsdGeomSubset>())
    {
      sdf::Visual visSubset;
      std::string nameMaterial = ParseMaterialName(child);
      auto it = _usdData.Materials().find(nameMaterial);
      if (it != _usdData.Materials().end())
      {
        visSubset.SetMaterial(it->second);
      }

      gz::common::Mesh meshSubset;

      numSubMeshes++;

      gz::common::SubMesh subMeshSubset;
      subMeshSubset.SetPrimitiveType(gz::common::SubMesh::TRISTRIPS);
      subMeshSubset.SetName("subgeommesh_" + std::to_string(numSubMeshes));

      if (it != _usdData.Materials().end())
      {
        std::shared_ptr<gz::common::Material> matCommon =
          std::make_shared<gz::common::Material>();
        convert(it->second, *matCommon.get());
        meshSubset.AddMaterial(matCommon);
        subMeshSubset.SetMaterialIndex(meshSubset.MaterialCount() - 1);
      }
      pxr::VtIntArray faceVertexIndices;
      child.GetAttribute(pxr::TfToken("indices")).Get(&faceVertexIndices);

      for (unsigned int i = 0; i < faceVertexIndices.size(); ++i)
      {
        subMeshSubset.AddIndex(_subMesh.Index(faceVertexIndices[i] * 3));
        subMeshSubset.AddIndex(_subMesh.Index(faceVertexIndices[i] * 3 + 1));
        subMeshSubset.AddIndex(_subMesh.Index(faceVertexIndices[i] * 3 + 2));
      }

      for (unsigned int i = 0; i < _subMesh.VertexCount(); ++i)
      {
        subMeshSubset.AddVertex(_subMesh.Vertex(i));
      }
      for (unsigned int i = 0; i < _subMesh.NormalCount(); ++i)
      {
        subMeshSubset.AddNormal(_subMesh.Normal(i));
      }
      for (unsigned int i = 0; i < _subMesh.TexCoordCount(); ++i)
      {
        subMeshSubset.AddTexCoord(_subMesh.TexCoord(i));
      }

      meshSubset.AddSubMesh(subMeshSubset);
      sdf::Mesh meshGeomSubset;
      sdf::Geometry geomSubset;
      geomSubset.SetType(sdf::GeometryType::MESH);

      std::string childPathName = pxr::TfStringify(child.GetPath());
      std::string directoryMesh = directoryFromUSDPath(childPathName) +
        childPathName;

      std::string meshFile;
      auto errorsExport = exportMesh(
        meshSubset, directoryMesh, _usdData, meshFile);
      if (!errorsExport.empty())
      {
        _errors.insert(_errors.end(), errorsExport.begin(), errorsExport.end());
        continue;
      }
      _meshGeom.SetFilePath(meshFile);
      _meshGeom.SetUri(meshFile);

      geomSubset.SetMeshShape(_meshGeom);
      visSubset.SetName("mesh_subset_" + std::to_string(numSubMeshes));
      visSubset.SetGeom(geomSubset);

      math::Pose3d pose;
      math::Vector3d scale(1, 1, 1);
      std::string linkName = pxr::TfStringify(_prim.GetPath());
      auto found = linkName.find(_link->Name());
      if (found != std::string::npos)
      {
        linkName = linkName.substr(0, found + _link->Name().size());
      }
      GetTransform(child, _usdData, pose, scale, linkName);
      _scale *= scale;
      visSubset.SetRawPose(pose);
      _link->AddVisual(visSubset);
    }
  }
  return numSubMeshes;
}

//////////////////////////////////////////////////
/// \brief Parse USD mesh
/// \param[in] _prim Reference to the USD prim
/// \param[in] _link Current link
/// \param[in] _vis sdf visual
/// \param[in] _geom sdf geom
/// \param[in] _scale scale mesh
/// \param[in,out] _usdData metadata of the USD file
/// \param[out] _pose The pose of the parsed mesh
/// \return gz::usd::UsdErrors, which is a list of UsdError objects.
///  An empty list means that no errors occurred when parsing the USD mesh
gz::usd::UsdErrors ParseMesh(
  const pxr::UsdPrim &_prim,
  sdf::Link *_link,
  sdf::Visual &_vis,
  sdf::Geometry &_geom,
  math::Vector3d &_scale,
  USDData &_usdData,
  math::Pose3d &_pose)
{
  gz::usd::UsdErrors errors;

  const std::pair<std::string, std::shared_ptr<USDStage>> data =
    _usdData.FindStage(_prim.GetPath().GetName());

  double metersPerUnit = data.second->MetersPerUnit();

  gz::common::Mesh mesh;
  gz::common::SubMesh subMesh;
  subMesh.SetPrimitiveType(gz::common::SubMesh::TRISTRIPS);
  pxr::VtIntArray faceVertexIndices;
  pxr::VtIntArray faceVertexCounts;
  pxr::VtArray<pxr::GfVec3f> normals;
  pxr::VtArray<pxr::GfVec3f> points;
  pxr::VtArray<pxr::GfVec2f> textCoords;
  _prim.GetAttribute(pxr::TfToken("faceVertexCounts")).Get(&faceVertexCounts);
  _prim.GetAttribute(pxr::TfToken("faceVertexIndices")).Get(&faceVertexIndices);
  _prim.GetAttribute(pxr::TfToken("normals")).Get(&normals);
  _prim.GetAttribute(pxr::TfToken("points")).Get(&points);
  _prim.GetAttribute(pxr::TfToken("primvars:st")).Get(&textCoords);

  if (textCoords.size() == 0)
  {
    _prim.GetAttribute(pxr::TfToken("primvars:st_0")).Get(&textCoords);
  }

  std::vector<unsigned int> indexes;
  errors = PolygonToTriangles(
    faceVertexIndices, faceVertexCounts, indexes);
  if (!errors.empty())
  {
    errors.emplace_back(UsdErrorCode::USD_TO_GZ_USD_POLYGON_PARSING_ERROR,
      "Unable to parse polygon in path [" + pxr::TfStringify(_prim.GetPath())
      + "]");
    return errors;
  }

  for (unsigned int i = 0; i < indexes.size(); ++i)
  {
    subMesh.AddIndex(indexes[i]);
  }

  for (const auto & textCoord : textCoords)
  {
    subMesh.AddTexCoord(textCoord[0], (1 - textCoord[1]));
  }

  for (const auto & point : points)
  {
    math::Vector3d v =
      math::Vector3d(point[0], point[1], point[2]) * metersPerUnit;
    subMesh.AddVertex(v);
  }

  for (const auto & normal : normals)
  {
    subMesh.AddNormal(normal[0], normal[1], normal[2]);
  }

  sdf::Mesh meshGeom;
  _geom.SetType(sdf::GeometryType::MESH);

  math::Pose3d pose;
  math::Vector3d scale(1, 1, 1);
  std::string linkName = pxr::TfStringify(_prim.GetPath());
  auto found = linkName.find(_link->Name());
  if (found != std::string::npos)
  {
    linkName = linkName.substr(0, found + _link->Name().size());
  }

  size_t nSlash = std::count(linkName.begin(), linkName.end(), '/');
  if (nSlash == 1)
  {
    GetTransform(_prim, _usdData, pose, scale, "/");
  }
  else
  {
    GetTransform(_prim, _usdData, pose, scale, linkName);
  }

  _pose = pose;

  meshGeom.SetScale(scale * _scale);

  std::string primName = pxr::TfStringify(_prim.GetPath());

  std::string directoryMesh = directoryFromUSDPath(primName) + primName;

  meshGeom.SetFilePath(
    gz::common::joinPaths(
      directoryMesh, gz::common::basename(directoryMesh)) + ".dae");

  meshGeom.SetUri(meshGeom.FilePath());

  int numSubMeshes = ParseMeshSubGeom(
    _prim, _link, subMesh, meshGeom, _scale, _usdData, errors);
  if (!errors.empty())
  {
    return errors;
  }

  _geom.SetMeshShape(meshGeom);

  if (numSubMeshes == 0)
  {
    std::string nameMaterial = ParseMaterialName(_prim);
    auto it = _usdData.Materials().find(nameMaterial);
    if (it != _usdData.Materials().end())
    {
      _vis.SetMaterial(it->second);
      std::shared_ptr<gz::common::Material> matCommon =
        std::make_shared<gz::common::Material>();
      convert(it->second, *matCommon.get());
      mesh.AddMaterial(matCommon);
      subMesh.SetMaterialIndex(mesh.MaterialCount() - 1);
    }

    mesh.AddSubMesh(subMesh);

    std::string meshFile;
    errors = exportMesh(mesh, directoryMesh, _usdData, meshFile);
    if (!errors.empty())
    {
      return errors;
    }
    meshGeom.SetFilePath(meshFile);
    meshGeom.SetUri(meshFile);
    _geom.SetMeshShape(meshGeom);

    std::string primNameStr = _prim.GetPath().GetName();
    _vis.SetName(primNameStr + "_visual");
    _vis.SetRawPose(pose);
    _vis.SetGeom(_geom);

    _link->AddVisual(_vis);
  }

  return errors;
}

//////////////////////////////////////////////////
/// \brief Parse USD cube
/// \param[in] _prim Reference to the USD prim
/// \param[in] _geom sdf geom
/// \param[in] _scale scale mesh
/// \param[in] _metersPerUnit meter per unit of the stage
void ParseCube(const pxr::UsdPrim &_prim,
  sdf::Geometry &_geom,
  const math::Vector3d &_scale,
  double _metersPerUnit)
{
  double size;
  auto variant_cube = pxr::UsdGeomCube(_prim);
  variant_cube.GetSizeAttr().Get(&size);

  size = size * _metersPerUnit;

  sdf::Box box;
  _geom.SetType(sdf::GeometryType::BOX);
  box.SetSize(math::Vector3d(
    size * _scale.X(),
    size * _scale.Y(),
    size * _scale.Z()));

  _geom.SetBoxShape(box);
}

//////////////////////////////////////////////////
/// \brief Parse USD sphere
/// \param[in] _prim Reference to the USD prim
/// \param[in] _geom sdf geom
/// \param[in] _scale scale mesh
/// \param[in] _metersPerUnit meter per unit of the stage
void ParseSphere(const pxr::UsdPrim &_prim,
  sdf::Geometry &_geom,
  const math::Vector3d &_scale,
  double _metersPerUnit)
{
  double radius;
  auto variant_sphere = pxr::UsdGeomSphere(_prim);
  variant_sphere.GetRadiusAttr().Get(&radius);

  sdf::Sphere s;
  _geom.SetType(sdf::GeometryType::SPHERE);
  s.SetRadius(radius * _metersPerUnit * _scale.X());
  _geom.SetSphereShape(s);
}

//////////////////////////////////////////////////
/// \brief Parse USD cone
/// \param[in] _prim Reference to the USD prim
/// \param[in] _geom sdf geom
/// \param[in] _scale scale mesh
/// \param[in] _metersPerUnit meter per unit of the stage
void ParseCone(
  const pxr::UsdPrim &_prim,
  sdf::Geometry &_geom,
  const math::Vector3d &_scale,
  double _metersPerUnit)
{
  auto variant_cone = pxr::UsdGeomCone(_prim);
  double radius;
  double height;
  variant_cone.GetRadiusAttr().Get(&radius);
  variant_cone.GetHeightAttr().Get(&height);

  sdf::Cone c;
  _geom.SetType(sdf::GeometryType::CONE);

  c.SetRadius(radius * _metersPerUnit * _scale.X());
  c.SetLength(height * _metersPerUnit * _scale.Z());

  _geom.SetConeShape(c);
}

//////////////////////////////////////////////////
/// \brief Parse USD cylinder
/// \param[in] _prim Reference to the USD prim
/// \param[in] _geom sdf geom
/// \param[in] _scale scale mesh
/// \param[in] _metersPerUnit meter per unit of the stage
void ParseCylinder(
  const pxr::UsdPrim &_prim,
  sdf::Geometry &_geom,
  const math::Vector3d &_scale,
  double _metersPerUnit)
{
  auto variant_cylinder = pxr::UsdGeomCylinder(_prim);
  double radius;
  double height;
  variant_cylinder.GetRadiusAttr().Get(&radius);
  variant_cylinder.GetHeightAttr().Get(&height);

  sdf::Cylinder c;
  _geom.SetType(sdf::GeometryType::CYLINDER);

  c.SetRadius(radius * _metersPerUnit * _scale.X());
  c.SetLength(height * _metersPerUnit * _scale.Z());

  _geom.SetCylinderShape(c);
}

//////////////////////////////////////////////////
gz::usd::UsdErrors ParseUSDLinks(
  const pxr::UsdPrim &_prim,
  const std::string &_nameLink,
  std::optional<sdf::Link> &_link,
  USDData &_usdData,
  math::Vector3d &_scale)
{
  gz::usd::UsdErrors errors;
  const std::string primNameStr = _prim.GetPath().GetName();
  const std::string primPathStr = pxr::TfStringify(_prim.GetPath());
  const std::string primType = _prim.GetPrimTypeInfo().GetTypeName().GetText();

  const std::pair<std::string, std::shared_ptr<USDStage>> data =
    _usdData.FindStage(primNameStr);

  // Is this a new link ?
  if (!_link)
  {
    _link = sdf::Link();
    _link->SetName(gz::common::basename(_nameLink));

    // USD define visual inside other visuals or links
    // This loop allow to find the link for a specific visual
    // For example
    // This visual /ur10_long_suction/shoulder_link/cylinder
    // correponds to the link /ur10_long_suction/wrist_3_link
    // Then we can find the right transformations
    std::string originalPrimName = pxr::TfStringify(_prim.GetPath());
    size_t pos = std::string::npos;
    std::string nameOfLink;
    if ((pos = originalPrimName.find(_link->Name()))!= std::string::npos)
    {
      nameOfLink = originalPrimName.erase(
        pos + _link->Name().length(),
        originalPrimName.length() - (pos + _link->Name().length()));
    }
    pxr::UsdPrim tmpPrim = _prim;
    if (!nameOfLink.empty())
    {
      while (tmpPrim)
      {
        if (pxr::TfStringify(tmpPrim.GetPath()) == nameOfLink)
        {
          break;
        }
        tmpPrim = tmpPrim.GetParent();
      }
    }

    math::Pose3d pose;
    math::Vector3d scale(1, 1, 1);
    GetTransform(tmpPrim, _usdData, pose, scale, "");
    // This is a special case when a geometry is defined in the higher level
    // of the path. we should only set the position if the path at least has
    // more than 1 level.
    // TODO(ahcorde) Review this code and improve this logic
    size_t nSlash = std::count(_nameLink.begin(), _nameLink.end(), '/');
    if (nSlash > 1)
      _link->SetRawPose(pose);
    _scale *= scale;
  }

  // If the schema is a rigid body use this name instead.
  if (_prim.HasAPI<pxr::UsdPhysicsRigidBodyAPI>())
  {
    _link->SetName(gz::common::basename(primPathStr));
  }

  math::Inertiald noneInertial = {{1.0,
            math::Vector3d::One, math::Vector3d::Zero},
            math::Pose3d::Zero};
  const auto inertial = _link->Inertial();
  if (inertial == noneInertial)
  {
    math::Inertiald newInertial;
    GetInertial(_prim, newInertial);
    _link->SetInertial(newInertial);
  }

  sdf::Geometry geom;
  if (_prim.IsA<pxr::UsdGeomSphere>() ||
      _prim.IsA<pxr::UsdGeomCone>() ||
      _prim.IsA<pxr::UsdGeomCylinder>() ||
      _prim.IsA<pxr::UsdGeomCube>() ||
      _prim.IsA<pxr::UsdGeomMesh>() ||
      primType == "Plane")
  {
    sdf::Visual vis;

    auto variant_geom = pxr::UsdGeomGprim(_prim);

    bool collisionEnabled = false;
    if (_prim.HasAPI<pxr::UsdPhysicsCollisionAPI>())
    {
      _prim.GetAttribute(
        pxr::TfToken("physics:collisionEnabled")).Get(&collisionEnabled);
    }

    pxr::TfToken kindOfSchema;
    if(!pxr::UsdModelAPI(_prim).GetKind(&kindOfSchema))
    {
      auto parent = _prim.GetParent();
      pxr::UsdModelAPI(parent).GetKind(&kindOfSchema);
    }

    if ((_prim.HasAPI<pxr::UsdPhysicsRigidBodyAPI>()
      || pxr::KindRegistry::IsA(kindOfSchema, pxr::KindTokens->model))
      && (!collisionEnabled || _prim.HasAPI<pxr::UsdPhysicsMassAPI>()))
    {
      double metersPerUnit = data.second->MetersPerUnit();

      if (_prim.IsA<pxr::UsdGeomSphere>())
      {
        ParseSphere(_prim, geom, _scale, metersPerUnit);
        vis.SetName("visual_sphere");
        vis.SetGeom(geom);
        _link->AddVisual(vis);
      }
      else if (_prim.IsA<pxr::UsdGeomCone>())
      {
        ParseCone(_prim, geom, _scale, metersPerUnit);
        vis.SetName("visual_cone");
        vis.SetGeom(geom);
        _link->AddVisual(vis);
      }
      else if (_prim.IsA<pxr::UsdGeomCylinder>())
      {
        ParseCylinder(_prim, geom, _scale, metersPerUnit);
        vis.SetName("visual_cylinder");
        vis.SetGeom(geom);
        _link->AddVisual(vis);
      }
      else if (_prim.IsA<pxr::UsdGeomCube>())
      {
        ParseCube(_prim, geom, _scale, metersPerUnit);
        vis.SetName("visual_box");
        vis.SetGeom(geom);
        _link->AddVisual(vis);
      }
      else if (_prim.IsA<pxr::UsdGeomMesh>())
      {
        math::Pose3d poseTmp;
        errors = ParseMesh(
          _prim, &_link.value(), vis, geom, _scale, _usdData, poseTmp);
        if (!errors.empty())
        {
          errors.emplace_back(UsdError(
          gz::usd::UsdErrorCode::GZ_USD_TO_USD_PARSING_ERROR,
            "Error parsing mesh"));
          return errors;
        }
      }
    }

    pxr::TfTokenVector schemasCollision = _prim.GetAppliedSchemas();
    bool physxCollisionAPIenable = false;
    for (const auto & token : schemasCollision)
    {
      if (std::string(token.GetText()) == "PhysxCollisionAPI")
      {
        physxCollisionAPIenable = true;
        break;
      }
    }

    if (collisionEnabled || physxCollisionAPIenable)
    {
      sdf::Collision col;

      // add _collision extension
      std::string collisionName = _prim.GetPath().GetName() + "_collision";
      col.SetName(collisionName);
      sdf::Geometry colGeom;

      math::Pose3d poseCol;
      math::Vector3d scaleCol(1, 1, 1);
      std::string linkName = pxr::TfStringify(_prim.GetPath());
      auto found = linkName.find(_link->Name());
      if (found != std::string::npos)
      {
        linkName = linkName.substr(0, found + _link->Name().size());
      }
      GetTransform(_prim, _usdData, poseCol, scaleCol, linkName);

      scaleCol *= _scale;

      double metersPerUnit = data.second->MetersPerUnit();

      if (_prim.IsA<pxr::UsdGeomSphere>())
      {
        ParseSphere(_prim, colGeom, scaleCol, metersPerUnit);
        col.SetGeom(colGeom);
        col.SetRawPose(poseCol);
      }
      else if (_prim.IsA<pxr::UsdGeomCone>())
      {
        ParseCone(_prim, colGeom, scaleCol, metersPerUnit);
        col.SetGeom(colGeom);
        col.SetRawPose(poseCol);
      }
      else if (_prim.IsA<pxr::UsdGeomCylinder>())
      {
        ParseCylinder(_prim, colGeom, scaleCol, metersPerUnit);
        col.SetGeom(colGeom);
        col.SetRawPose(poseCol);
      }
      else if (_prim.IsA<pxr::UsdGeomCube>())
      {
        ParseCube(_prim, colGeom, scaleCol, metersPerUnit);
        col.SetGeom(colGeom);
        col.SetRawPose(poseCol);
      }
      else if (_prim.IsA<pxr::UsdGeomMesh>())
      {
        sdf::Visual visTmp;
        math::Pose3d poseTmp;
        errors = ParseMesh(
          _prim, &_link.value(), visTmp, colGeom, scaleCol, _usdData, poseTmp);
        if (!errors.empty())
        {
          errors.emplace_back(UsdError(
          gz::usd::UsdErrorCode::GZ_USD_TO_USD_PARSING_ERROR,
            "Error parsing mesh"));
          return errors;
        }
        col.SetRawPose(poseTmp);
        col.SetGeom(colGeom);
      }
      else if (primType == "Plane")
      {
        sdf::Plane plane;
        colGeom.SetType(sdf::GeometryType::PLANE);
        plane.SetSize(math::Vector2d(100, 100));
        colGeom.SetPlaneShape(plane);

        math::Pose3d pose;
        math::Vector3d scale(1, 1, 1);
        GetTransform(
          _prim, _usdData, pose, scale, pxr::TfStringify(_prim.GetPath()));
        col.SetRawPose(pose);
        col.SetGeom(colGeom);
      }

      _link->AddCollision(col);
    }

  }
  return errors;
}
}
}
}
