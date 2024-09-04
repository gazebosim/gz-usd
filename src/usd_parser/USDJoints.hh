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

#ifndef USD_PARSER_JOINTS_HH
#define USD_PARSER_JOINTS_HH

#pragma push_macro ("__DEPRECATED")
#undef __DEPRECATED
#include <pxr/usd/usdGeom/gprim.h>
#pragma pop_macro ("__DEPRECATED")

#include "sdf/Joint.hh"
#include "gz/usd/usd_parser/USDData.hh"
#include <gz/usd/config.hh>

namespace gz
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_USD_VERSION_NAMESPACE {
  //
  namespace usd
  {
    /// \brief Parse a USD joint to its SDF representation
    /// This method will parse revolute and prismatic joints, if the Joint type
    /// is not one of these two, then the joint is fixed. This will help to
    /// avoid issues with when two or more links links in a model doesn't have
    /// a joint
    ///
    /// \param[in] _prim The USD prim that holds the USD joint
    /// \param[in] _usdData Object that holds data about the USD stage
    /// \param[out] _joint SDF joint to return
    /// \return gz::usd::UsdErrors, which is a list of UsdError objects.
    /// An empty list means there were no errors parsing joint
    gz::usd::UsdErrors GZ_USD_VISIBLE ParseJoints(
      const pxr::UsdPrim &_prim,
      const USDData &_usdData,
      sdf::Joint &_joint);
  }
  }
}

#endif
