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

#include "Material.hh"

#include <map>
#include <pxr/usd/usdShade/shader.h>
#include <string>

#include <ignition/common/Filesystem.hh>
#include <ignition/common/Util.hh>
#include <ignition/common/URI.hh>

#include <ignition/math/Color.hh>
#include <utility>

// TODO(ahcorde) this is to remove deprecated "warnings" in usd, these warnings
// are reported using #pragma message so normal diagnostic flags cannot remove
// them. This workaround requires this block to be used whenever usd is
// included.
#pragma push_macro ("__DEPRECATED")
#undef __DEPRECATED
#include <pxr/base/tf/stringUtils.h>
#include <pxr/usd/sdf/types.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdShade/connectableAPI.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#pragma pop_macro ("__DEPRECATED")

#include "sdf/Pbr.hh"

namespace gz
{
// Inline bracket to help doxygen filtering.
inline namespace GZ_USD_VERSION_NAMESPACE {
//
namespace usd
{

  /// \brief Get the path to copy the material to
  /// \param[in] _uri full path of the file to copy
  /// \return A relative path to save the material. The path looks like:
  /// materials/textures/<filename with extension>
  std::string getMaterialCopyPath(const std::string &_uri)
  {
    return ignition::common::joinPaths(
      "materials",
      "textures",
      ignition::common::basename(_uri));
  }

  /// \brief Copy a file in a directory
  /// \param[in] _path path where the copy will be located
  /// \param[in] _fullPath name of the file to copy
  /// \return True if the file at _fullPath was copied to _path. False otherwise
std::pair<bool, std::string> copyMaterial(const std::string &_sdfMaterialPath)
  {
    std::string copyPath = getMaterialCopyPath(_sdfMaterialPath);
    std::string fullPath = common::findFile(_sdfMaterialPath);

    if (fullPath.empty())
    {
      fullPath = _sdfMaterialPath;
    }

    if (!copyPath.empty() && !fullPath.empty())
    {
      auto fileName = ignition::common::basename(copyPath);
      auto filePathIndex = copyPath.rfind(fileName);
      auto filePath = copyPath.substr(0, filePathIndex);
      if (!filePath.empty())
      {
        ignition::common::createDirectories(filePath);
      }
      return {ignition::common::copyFile(fullPath, copyPath), copyPath};
    }
    return {false, ""};
  }
  /// \brief Fill Material shader attributes and properties
  /// \param[in] _prim USD primitive
  /// \param[in] _name Name of the field attribute or property
  /// \param[in] _vType Type of the field
  /// \param[in] _value Value of the field
  /// \param[in] _customData Custom data to set the field
  /// \param[in] _displayName Display name
  /// \param[in] _displayGroup Display group
  /// \param[in] _doc Documentation of the field
  /// \param[in] _colorSpace if the material is a texture, we can specify the
  /// color space of the image
  /// \return gz::usd::UsdErrors, which is a list of UsdError objects.
  /// This list is empty if no errors occurred when creating the material input.
  template<typename T>
  gz::usd::UsdErrors CreateMaterialInput(
    const pxr::UsdPrim &_prim,
    const std::string &_name,
    const pxr::SdfValueTypeName &_vType,
    const T &_value,
    const std::map<pxr::TfToken, pxr::VtValue> &_customData,
    const pxr::TfToken &_displayName,
    const pxr::TfToken &_displayGroup,
    const std::string &_doc,
    const pxr::TfToken &_colorSpace = pxr::TfToken(""))
  {
    gz::usd::UsdErrors errors;
    auto shader = pxr::UsdShadeShader(_prim);
    if (!shader)
    {
      errors.emplace_back(UsdError(
        gz::usd::UsdErrorCode::INVALID_PRIM_PATH,
        "Unable to convert the prim to a UsdShadeShader"));
      return errors;
    }

    auto existingInput = shader.GetInput(pxr::TfToken(_name));
    pxr::SdfValueTypeName vTypeName;
    if (_vType.IsScalar())
    {
      vTypeName = _vType.GetScalarType();
    }
    else if (_vType.IsArray())
    {
      vTypeName = _vType.GetArrayType();
    }
    auto surfaceInput = shader.CreateInput(
      pxr::TfToken(_name), vTypeName);
    surfaceInput.Set(_value);
    auto attr = surfaceInput.GetAttr();

    for (const auto &[key, customValue] : _customData)
    {
      attr.SetCustomDataByKey(key, customValue);
    }
    if (!_displayName.GetString().empty())
    {
      attr.SetDisplayName(_displayName);
    }
    if (!_displayGroup.GetString().empty())
    {
      attr.SetDisplayGroup(_displayGroup);
    }
    if (!_doc.empty())
    {
      attr.SetDocumentation(_doc);
    }
    if (!_colorSpace.GetString().empty())
    {
      attr.SetColorSpace(_colorSpace);
    }
    return errors;
  }

  void CreateSurfaceTextureShader(pxr::UsdStageRefPtr &_stage,
                                  pxr::UsdShadeShader &_shader,
                                  const pxr::SdfValueTypeName &_vType,
                                  const std::string &_textureType,
                                  const std::string &_texturePath,
                                  const pxr::TfToken &_outputName)
  {
    const auto primTexture = _stage->DefinePrim(
        _shader.GetPath().GetParentPath().AppendPath(pxr::SdfPath(_textureType + "Texture")),
        pxr::TfToken("Shader"));
    pxr::UsdShadeShader shaderTexture(primTexture);
    shaderTexture.CreateIdAttr().Set(pxr::TfToken("UsdUVTexture"));
    shaderTexture
        .CreateInput(pxr::TfToken("file"), pxr::SdfValueTypeNames->Asset)
        .Set(pxr::SdfAssetPath(_texturePath));
    auto output =
        pxr::UsdShadeConnectableAPI(primTexture)
            .CreateOutput(_outputName, pxr::SdfValueTypeNames->Token);
    _shader
        .CreateInput(pxr::TfToken(_textureType), _vType)
        .ConnectToSource(output);
  }

  gz::usd::UsdErrors ParseSdfMaterial(const sdf::Material *_materialSdf,
      pxr::UsdStageRefPtr &_stage, pxr::SdfPath &_materialPath)
  {
    gz::usd::UsdErrors errors;
    common::systemPaths()->AddFilePaths(common::parentPath(_materialSdf->FilePath()));


    const auto looksPath = pxr::SdfPath("/Looks");
    auto looksPrim = _stage->GetPrimAtPath(looksPath);
    if (!looksPrim)
    {
      looksPrim = _stage->DefinePrim(looksPath, pxr::TfToken("Scope"));
    }

    // This variable will increase with every new material to avoid collision
    // with the names of the materials
    static int i = 0;

    _materialPath = pxr::SdfPath("/Looks/Material_" + std::to_string(i));
    i++;

    pxr::UsdShadeMaterial materialUsd;
    auto usdMaterialPrim = _stage->GetPrimAtPath(_materialPath);
    if (!usdMaterialPrim)
    {
      materialUsd = pxr::UsdShadeMaterial::Define(_stage, _materialPath);
    }
    else
    {
      materialUsd = pxr::UsdShadeMaterial(usdMaterialPrim);
    }

    const auto shaderPath = pxr::SdfPath(_materialPath.GetString() + "/PbrShader");
    auto usdShader = pxr::UsdShadeShader::Define(_stage, shaderPath);
    usdShader.CreateIdAttr().Set(pxr::TfToken("UsdPreviewSurface"));
    auto shaderPrim = _stage->GetPrimAtPath(shaderPath);
    if (!shaderPrim)
    {
      errors.emplace_back(UsdError(
        gz::usd::UsdErrorCode::INVALID_PRIM_PATH,
        "Not able to cast the UsdShadeShader at path [" + shaderPath.GetString()
        + "] to a Prim"));
      return errors;
    }

    const math::Color diffuse = _materialSdf->Diffuse();
    const math::Color emissive = _materialSdf->Emissive();
    bool haveAlebedoMap = false;
    bool haveRoughnessMap = false;
    usdShader.CreateInput(pxr::TfToken("emissiveColor"), pxr::SdfValueTypeNames->Color3f).Set(pxr::GfVec3f(emissive.R(), emissive.G(), emissive.B()));

    auto shaderOut = pxr::UsdShadeConnectableAPI(shaderPrim).CreateOutput(
      pxr::TfToken("surface"), pxr::SdfValueTypeNames->Token);
    materialUsd.CreateSurfaceOutput().ConnectToSource(shaderOut);

    const sdf::Pbr * pbr = _materialSdf->PbrMaterial();
    if (pbr)
    {
      const sdf::PbrWorkflow * pbrWorkflow =
        pbr->Workflow(sdf::PbrWorkflowType::METAL);
      if (!pbrWorkflow)
      {
        if (pbr->Workflow(sdf::PbrWorkflowType::SPECULAR))
        {
          std::cout << "Only metalness workflow supported\n";
        }
        return errors;
      }

      if (pbrWorkflow)
      {
        usdShader.CreateInput(pxr::TfToken("metallic"), pxr::SdfValueTypeNames->Float).Set(static_cast<float>(pbrWorkflow->Metalness()));
        usdShader.CreateInput(pxr::TfToken("roughness"), pxr::SdfValueTypeNames->Float).Set(static_cast<float>(pbrWorkflow->Roughness()));

        const std::map<pxr::TfToken, pxr::VtValue> customDefaultSdfAssetPath =
        {
          {pxr::TfToken("default"), pxr::VtValue(pxr::SdfAssetPath())},
        };

        if (!pbrWorkflow->AlbedoMap().empty())
        {
          auto [copied, copyPath] = copyMaterial(pbrWorkflow->AlbedoMap());
          CreateSurfaceTextureShader(
              _stage, usdShader, pxr::SdfValueTypeNames->Color3f,
              "diffuseColor", copyPath, pxr::TfToken("rgb"));
          haveAlebedoMap = true;
        }
        if (!pbrWorkflow->MetalnessMap().empty())
        {
          auto [copied, copyPath] = copyMaterial(pbrWorkflow->MetalnessMap());
          CreateSurfaceTextureShader(_stage, usdShader,
                                     pxr::SdfValueTypeNames->Float, "metallic",
                                     copyPath, pxr::TfToken("r"));
        }
        if (!pbrWorkflow->NormalMap().empty())
        {
          auto [copied, copyPath] = copyMaterial(pbrWorkflow->NormalMap());
          CreateSurfaceTextureShader(_stage, usdShader,
                                     pxr::SdfValueTypeNames->Color3f, "normal",
                                     copyPath, pxr::TfToken("rgb"));
        }
        if (!pbrWorkflow->RoughnessMap().empty())
        {
          auto [copied, copyPath] = copyMaterial(pbrWorkflow->RoughnessMap());
          CreateSurfaceTextureShader(_stage, usdShader,
                                     pxr::SdfValueTypeNames->Float, "roughness",
                                     copyPath, pxr::TfToken("r"));
          haveRoughnessMap = true;
        }
      }

      if (!haveAlebedoMap)
        usdShader.CreateInput(pxr::TfToken("diffuseColor"), pxr::SdfValueTypeNames->Color3f).Set(pxr::GfVec3f(diffuse.R(), diffuse.G(), diffuse.B()));
      if (!haveRoughnessMap)
        usdShader.CreateInput(pxr::TfToken("roughness"), pxr::SdfValueTypeNames->Float).Set(0.5f);
    }

    return errors;
  }
}
}
}
