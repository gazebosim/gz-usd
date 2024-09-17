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

#include <functional>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include <gz/utils/ExtraTestMacros.hh>

#include "test_config.h"
#include "test_utils.hh"

// TODO(ahcorde) this is to remove deprecated "warnings" in usd, these warnings
// are reported using #pragma message so normal diagnostic flags cannot remove
// them. This workaround requires this block to be used whenever usd is
// included.
#pragma push_macro ("__DEPRECATED")
#undef __DEPRECATED
#include <pxr/usd/usd/stage.h>
#pragma pop_macro ("__DEPRECATED")

#include "gz/usd/usd_parser/USDTransforms.hh"

using namespace gz;

void checkTransforms(
  const std::string &_primName,
  pxr::UsdStageRefPtr &_stage,
  const math::Vector3d &_translation,
  const std::optional<math::Quaterniond> &_rotation,
  const math::Vector3d &_scale)
{
  pxr::UsdPrim prim = _stage->GetPrimAtPath(pxr::SdfPath(_primName));
  ASSERT_TRUE(prim);

  gz::usd::UDSTransforms usdTransforms =
    gz::usd::ParseUSDTransform(prim);

  EXPECT_EQ(_translation, usdTransforms.Translation());
  EXPECT_EQ(_scale, usdTransforms.Scale());
  if (_rotation)
  {
    ASSERT_TRUE(usdTransforms.Rotation());
    EXPECT_TRUE(math::equal(
      _rotation.value().Roll(),
       usdTransforms.Rotation().value().Roll(), 0.1));
   EXPECT_TRUE(math::equal(
     _rotation.value().Pitch(),
      usdTransforms.Rotation().value().Pitch(), 0.1));
    EXPECT_TRUE(math::equal(
      _rotation.value().Yaw(),
       usdTransforms.Rotation().value().Yaw(), 0.1));
  }
}

/////////////////////////////////////////////////
TEST(USDTransformsTest, ParseUSDTransform)
{
  std::string filename = gz::testing::TestFile("usd", "upAxisZ.usda");
  auto stage = pxr::UsdStage::Open(filename);
  ASSERT_TRUE(stage);

  checkTransforms(
    "/ground_plane",
    stage,
    math::Vector3d(0, 0, -0.125),
    math::Quaterniond(1, 0, 0, 0),
    math::Vector3d(1, 1, 1)
  );

  checkTransforms(
    "/ground_plane/link/visual/geometry",
    stage,
    math::Vector3d(0, 0, 0),
    std::nullopt,
    math::Vector3d(100, 100, 0.25)
  );

  checkTransforms(
    "/cylinder",
    stage,
    math::Vector3d(0, -1.5, 0.5),
    math::Quaterniond(1, 0, 0, 0),
    math::Vector3d(1, 1, 1)
  );

  checkTransforms(
    "/sphere",
    stage,
    math::Vector3d(0, 1.5, 0.5),
      math::Quaterniond(
        GZ_DTOR(-62), GZ_DTOR(-47.5), GZ_DTOR(-53.41)),
    math::Vector3d(1, 1, 1)
  );

  checkTransforms(
    "/capsule",
    stage,
    math::Vector3d(0, -3.0, 0.5),
    math::Quaterniond(
      GZ_DTOR(-75.1), GZ_DTOR(49.2), GZ_DTOR(-81.2)),
    math::Vector3d(1, 1, 1)
  );

  checkTransforms(
    "/capsule/capsule_link/capsule_visual",
    stage,
    math::Vector3d(0, 0, 0),
    math::Quaterniond(0, 0, M_PI_2),
    math::Vector3d(1, 1, 1)
  );

  checkTransforms(
    "/ellipsoid",
    stage,
    math::Vector3d(0, 3.0, 0.5),
    math::Quaterniond(
      GZ_DTOR(-75.1), GZ_DTOR(49.2), GZ_DTOR(-81.2)),
    math::Vector3d(1, 1, 1)
  );

  checkTransforms(
    "/ellipsoid/ellipsoid_link/ellipsoid_visual/geometry",
    stage,
    math::Vector3d(0, 0, 0),
    std::nullopt,
    math::Vector3d(0.4, 0.6, 1)
  );

  checkTransforms(
    "/sun",
    stage,
    math::Vector3d(0, 0, 10),
      math::Quaterniond(0, GZ_DTOR(-35), 0),
    math::Vector3d(1, 1, 1)
  );
}

/////////////////////////////////////////////////
TEST(USDTransformsTest, GetAllTransform)
{
  {
    std::string filename = gz::testing::TestFile("usd", "upAxisZ.usda");
    auto stage = pxr::UsdStage::Open(filename);
    ASSERT_TRUE(stage);

    gz::usd::USDData usdData(filename);
    usdData.Init();

    pxr::UsdPrim prim = stage->GetPrimAtPath(
      pxr::SdfPath(
        "/ellipsoid/ellipsoid_link/ellipsoid_visual/geometry"));
    ASSERT_TRUE(prim);

    math::Pose3d pose;
    math::Vector3d scale{1, 1, 1};

    gz::usd::GetTransform(prim, usdData, pose, scale, "/shapes");

    EXPECT_EQ(math::Vector3d(0.4, 0.6, 1), scale);
    EXPECT_EQ(
      math::Pose3d(
        math::Vector3d(0, 0.03, 0.005),
        math::Quaterniond(
          GZ_DTOR(-75.1), GZ_DTOR(49.2), GZ_DTOR(-81.2))),
      pose);
  }

  {
    std::string filename =
      gz::testing::TestFile("usd", "nested_transforms.usda");
    auto stage = pxr::UsdStage::Open(filename);
    ASSERT_TRUE(stage);

    gz::usd::USDData usdData(filename);
    usdData.Init();

    std::function<void(
      const std::string &,
      const math::Vector3d &,
      const math::Quaterniond &)> verifyNestedTf =
      [&](const std::string &_path,
          const math::Vector3d &_posePrim,
          const math::Quaterniond &_qPrim)
      {
        pxr::UsdPrim prim = stage->GetPrimAtPath(pxr::SdfPath(_path));
        ASSERT_TRUE(prim);

        math::Pose3d pose;
        math::Vector3d scale{1, 1, 1};

        gz::usd::GetTransform(prim, usdData, pose, scale, "/transforms");

        EXPECT_EQ(math::Vector3d(1, 1, 1), scale);
        EXPECT_EQ(math::Pose3d(_posePrim, _qPrim), pose);
      };

    verifyNestedTf(
      "/transforms/nested_transforms_XYZ/child_transform",
        math::Vector3d(0.01, 0.01, 0),
        math::Quaterniond(0, 0, GZ_DTOR(90)));
    verifyNestedTf(
      "/transforms/nested_transforms_ZYX/child_transform",
      math::Vector3d(0.02, 0.0, 0),
      math::Quaterniond(GZ_DTOR(90), 0, 0));
  }
}
