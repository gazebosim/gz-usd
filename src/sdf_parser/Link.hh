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

#ifndef IGNITION_USD_USD_IGNITION_USD_PARSER_LINK_HH_
#define IGNITION_USD_USD_IGNITION_USD_PARSER_LINK_HH_

#include <string>

// TODO(ahcorde) this is to remove deprecated "warnings" in usd, these warnings
// are reported using #pragma message so normal diagnostic flags cannot remove
// them. This workaround requires this block to be used whenever usd is
// included.
#pragma push_macro ("__DEPRECATED")
#undef __DEPRECATED
#include <pxr/usd/usd/stage.h>
#pragma pop_macro ("__DEPRECATED")

#include "sdf/Link.hh"
#include "ignition/usd/UsdError.hh"
#include <ignition/usd/config.hh>

namespace ignition
{
  // Inline bracke to help doxygen filtering.
  inline namespace IGNITION_USD_VERSION_NAMESPACE {
  //
  namespace usd
  {
    /// \brief Parse an SDF link into a USD stage.
    /// \param[in] _link The SDF link to parse.
    /// \param[in] _stage The stage that should contain the USD representation
    /// of _link. This must be a valid, initialized stage.
    /// \param[in] _path The USD path of the parsed link in _stage, which must
    /// be a valid USD path.
    /// \param[in] _rigidBody Whether the link is a rigid body (i.e.,
    /// non-static) or not. True for rigid body, false otherwise
    /// \return ignition::usd::UsdErrors, which is a vector of UsdError objects. Each UsdError
    /// includes an error code and message. An empty vector indicates no errors
    /// occurred when parsing _link to its USD representation.
    ignition::usd::UsdErrors ParseSdfLink(const sdf::Link &_link,
        pxr::UsdStageRefPtr &_stage, const std::string &_path,
        bool _rigidBody);
  }
  }
}

#endif
