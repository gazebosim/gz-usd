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

#include <gtest/gtest.h>

#include <functional>
#include <optional>
#include <string>
#include <utility>

// TODO(ahcorde) this is to remove deprecated "warnings" in usd, these warnings
// are reported using #pragma message so normal diagnostic flags cannot remove
// them. This workaround requires this block to be used whenever usd is
// included.
#pragma push_macro ("__DEPRECATED")
#undef __DEPRECATED
#include <pxr/usd/usd/stage.h>
#pragma pop_macro ("__DEPRECATED")

#include <gz/common/Filesystem.hh>
#include <gz/common/StringUtils.hh>
#include <gz/common/TempDirectory.hh>

#include "test_config.h"
#include "test_utils.hh"

#include "USDLinks.hh"

#include "sdf/Collision.hh"
#include "sdf/Geometry.hh"
#include "sdf/Visual.hh"

using namespace gz;

/////////////////////////////////////////////////
TEST(USDLinksTest, LinksNameMassAndDiagonalMoments)
{
  const std::string filename = gz::testing::TestFile("usd", "upAxisZ.usda");
  const auto stage = pxr::UsdStage::Open(filename);
  ASSERT_TRUE(stage);

  gz::usd::USDData usdData(filename);
  usdData.Init();

  math::Vector3d scale(1, 1, 1);

  auto checkLink =
    [](const sdf::Link &_link,
       const std::string &_name,
       double mass,
       const math::Vector3d &_diagonalMoment,
       unsigned int _nvisual,
       sdf::GeometryType _type)
    {
      EXPECT_EQ(_name, _link.Name());
      EXPECT_NEAR(mass, _link.Inertial().MassMatrix().Mass(), 1e-4);
      EXPECT_EQ(
        _diagonalMoment,
        _link.Inertial().MassMatrix().PrincipalMoments());
      EXPECT_EQ(_nvisual, _link.VisualCount());
      if (_nvisual)
      {
        auto visual = _link.VisualByIndex(0);
        ASSERT_NE(nullptr, visual);
        auto geom = visual->Geom();
        ASSERT_NE(nullptr, geom);
        EXPECT_EQ(_type, geom->Type());
        auto collision = _link.CollisionByIndex(0);
        ASSERT_NE(nullptr, collision);
        auto geomCol = collision->Geom();
        ASSERT_NE(nullptr, geomCol);
      }
    };

  const auto boxLink = stage->GetPrimAtPath(pxr::SdfPath("/box/box_link"));
  ASSERT_TRUE(boxLink);

  std::optional<sdf::Link> linkSDF;
  gz::usd::ParseUSDLinks(
    boxLink, "/box/box_link", linkSDF, usdData, scale);

  EXPECT_EQ(math::Vector3d(1, 0.1, 1), scale);

  const auto boxLinkGeometry = stage->GetPrimAtPath(
    pxr::SdfPath("/box/box_link/box_visual/geometry"));
  ASSERT_TRUE(boxLinkGeometry);

  gz::usd::ParseUSDLinks(
    boxLinkGeometry, "/box/box_link", linkSDF, usdData, scale);

  const auto boxLinkCollision = stage->GetPrimAtPath(
    pxr::SdfPath("/box/box_link/box_visual/collision"));
  ASSERT_TRUE(boxLinkCollision);

  gz::usd::ParseUSDLinks(
    boxLinkCollision, "/box/box_link", linkSDF, usdData, scale);

  ASSERT_TRUE(linkSDF);
  checkLink(linkSDF.value(), "box_link", 1.0,
            math::Vector3d(0.1666, 0.1666, 0.1666),
            1u, sdf::GeometryType::BOX);

  const auto cylinderLink = stage->GetPrimAtPath(
    pxr::SdfPath("/cylinder/cylinder_link"));
  ASSERT_TRUE(cylinderLink);

  std::optional<sdf::Link> linkCylinderSDF;
  gz::usd::ParseUSDLinks(
    cylinderLink, "/cylinder/cylinder_link",
    linkCylinderSDF, usdData, scale);

  const auto cylinderLinkGeometry = stage->GetPrimAtPath(
    pxr::SdfPath("/cylinder/cylinder_link/cylinder_visual/geometry"));
  EXPECT_TRUE(cylinderLinkGeometry);

  gz::usd::ParseUSDLinks(
    cylinderLinkGeometry, "/cylinder/cylinder_link",
    linkCylinderSDF, usdData, scale);

  const auto cylinderLinkCollision = stage->GetPrimAtPath(
    pxr::SdfPath("/cylinder/cylinder_link/cylinder_visual/collision"));
  EXPECT_TRUE(cylinderLinkCollision);

  gz::usd::ParseUSDLinks(
    cylinderLinkCollision, "/cylinder/cylinder_link",
    linkCylinderSDF, usdData, scale);

  checkLink(linkCylinderSDF.value(), "cylinder_link", 1.7,
            math::Vector3d(0.1458, 0.1458, 0.125),
            1u, sdf::GeometryType::CYLINDER);

  const auto sphereLink = stage->GetPrimAtPath(
    pxr::SdfPath("/sphere/sphere_link"));
  ASSERT_TRUE(sphereLink);

  std::optional<sdf::Link> linkSphereSDF;
  gz::usd::ParseUSDLinks(
    sphereLink, "/sphere/sphere_link", linkSphereSDF, usdData, scale);

  const auto sphereLinkGeometry = stage->GetPrimAtPath(
    pxr::SdfPath("/sphere/sphere_link/sphere_visual/geometry"));
  EXPECT_TRUE(sphereLinkGeometry);

  gz::usd::ParseUSDLinks(
    sphereLinkGeometry, "/sphere/sphere_link",
    linkSphereSDF, usdData, scale);

  const auto sphereLinkCollision = stage->GetPrimAtPath(
    pxr::SdfPath("/sphere/sphere_link/sphere_visual/collision"));
  EXPECT_TRUE(sphereLinkCollision);

  gz::usd::ParseUSDLinks(
    sphereLinkCollision, "/sphere/sphere_link",
    linkSphereSDF, usdData, scale);

  ASSERT_TRUE(linkSphereSDF);
  checkLink(linkSphereSDF.value(), "sphere_link", 2,
            math::Vector3d(0.1, 0.1, 0.1),
            1u, sdf::GeometryType::SPHERE);

  const auto capsuleLink = stage->GetPrimAtPath(
    pxr::SdfPath("/capsule/capsule_link"));
  ASSERT_TRUE(capsuleLink);

  std::optional<sdf::Link> linkCapsuleSDF;
  gz::usd::ParseUSDLinks(
    capsuleLink, "/capsule/capsule_link", linkCapsuleSDF, usdData, scale);

  const auto capsuleLinkCollision = stage->GetPrimAtPath(
    pxr::SdfPath("/capsule/capsule_link/capsule_visual/collision"));
  EXPECT_TRUE(capsuleLinkCollision);

  gz::usd::ParseUSDLinks(
    capsuleLinkCollision, "/capsule/capsule_link",
    linkCapsuleSDF, usdData, scale);

  checkLink(linkCapsuleSDF.value(), "capsule_link", 1,
            math::Vector3d(0.074154, 0.074154, 0.018769),
            0u, sdf::GeometryType::EMPTY);

  const auto ellipsoidLink = stage->GetPrimAtPath(
    pxr::SdfPath("/ellipsoid/ellipsoid_link"));
  ASSERT_TRUE(ellipsoidLink);

  std::optional<sdf::Link> linkEllipsoidSDF;
  gz::usd::ParseUSDLinks(
    ellipsoidLink, "/ellipsoid/ellipsoid_link",
    linkEllipsoidSDF, usdData, scale);

  const auto ellipsoidLinkCollision = stage->GetPrimAtPath(
    pxr::SdfPath("/ellipsoid/ellipsoid_link/ellipsoid_visual/collision"));
  EXPECT_TRUE(ellipsoidLinkCollision);

  gz::usd::ParseUSDLinks(
    ellipsoidLinkCollision, "/ellipsoid/ellipsoid_link",
    linkEllipsoidSDF, usdData, scale);

  checkLink(linkEllipsoidSDF.value(), "ellipsoid_link", 1,
            math::Vector3d(0.068, 0.058, 0.026),
            0u, sdf::GeometryType::EMPTY);
}

/////////////////////////////////////////////////
TEST(USDLinksTest, DuplicatedMeshesAreExportedOnce)
{
  const std::string filename =
    gz::testing::TestFile("usd", "duplicated_meshes.usda");
  const auto stage = pxr::UsdStage::Open(filename);
  ASSERT_TRUE(stage);

  gz::usd::USDData usdData(filename);
  EXPECT_TRUE(usdData.Init().empty());

  // Meshes are exported relative to the current working directory
  const std::string originalCwd = gz::common::cwd();
  const std::string tmp = gz::common::createTempDirectory("usd",
    gz::common::tempDirectoryPath());
  ASSERT_FALSE(tmp.empty());
  ASSERT_TRUE(gz::common::chdir(tmp));

  // Parse a mesh prim that has both a visual and a collision, and return
  // their mesh URIs
  auto parseMesh = [&](const std::string &_primPath,
                       const std::string &_linkPath)
    -> std::pair<std::string, std::string>
    {
      const auto prim = stage->GetPrimAtPath(pxr::SdfPath(_primPath));
      EXPECT_TRUE(prim);
      if (!prim)
        return {};

      std::optional<sdf::Link> link;
      math::Vector3d scale(1, 1, 1);
      EXPECT_TRUE(gz::usd::ParseUSDLinks(
        prim, _linkPath, link, usdData, scale).empty());
      EXPECT_TRUE(link);
      if (!link)
        return {};

      EXPECT_EQ(1u, link->VisualCount());
      EXPECT_EQ(1u, link->CollisionCount());
      if (link->VisualCount() != 1u || link->CollisionCount() != 1u)
        return {};

      const auto visGeom = link->VisualByIndex(0)->Geom();
      const auto colGeom = link->CollisionByIndex(0)->Geom();
      EXPECT_EQ(sdf::GeometryType::MESH, visGeom->Type());
      EXPECT_EQ(sdf::GeometryType::MESH, colGeom->Type());
      if (!visGeom->MeshShape() || !colGeom->MeshShape())
        return {};

      return {visGeom->MeshShape()->Uri(), colGeom->MeshShape()->Uri()};
    };

  const auto plant1 = parseMesh("/plant_1/leaves", "/plant_1");
  const auto plant2 = parseMesh("/plant_2/leaves", "/plant_2");
  const auto rock = parseMesh("/rock/stone", "/rock");

  // The visual and the collision of the same prim share the exported file,
  // and the file is not removed when the collision is parsed
  EXPECT_FALSE(plant1.first.empty());
  EXPECT_EQ(plant1.first, plant1.second);
  EXPECT_TRUE(gz::common::isFile(plant1.first));

  // A mesh with the same data reuses the file exported for the first one
  EXPECT_EQ(plant1.first, plant2.first);
  EXPECT_EQ(plant1.first, plant2.second);

  // A mesh with different data is exported to its own file
  EXPECT_FALSE(rock.first.empty());
  EXPECT_EQ(rock.first, rock.second);
  EXPECT_NE(plant1.first, rock.first);
  EXPECT_TRUE(gz::common::isFile(rock.first));

  // Only two Collada files were written to disk
  std::function<unsigned int(const std::string &)> countDae =
    [&countDae](const std::string &_dir)
    {
      unsigned int count = 0;
      for (gz::common::DirIter file(_dir);
           file != gz::common::DirIter(); ++file)
      {
        const std::string path = *file;
        if (gz::common::isDirectory(path))
        {
          count += countDae(path);
        }
        else if (gz::common::EndsWith(path, ".dae"))
        {
          ++count;
        }
      }
      return count;
    };
  EXPECT_EQ(2u, countDae(tmp));

  EXPECT_TRUE(gz::common::chdir(originalCwd));
  EXPECT_TRUE(gz::common::removeAll(tmp));
}
