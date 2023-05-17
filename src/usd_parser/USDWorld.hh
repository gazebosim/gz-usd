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

#ifndef USD_PARSER_USDWORLD_HH_
#define USD_PARSER_USDWORLD_HH_

#include <string>

#include <gz/usd/config.hh>
#include "gz/usd/UsdError.hh"

#include "sdf/World.hh"

namespace gz
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_USD_VERSION_NAMESPACE {
  //
  namespace usd
  {
    /// \brief Parse the world information of a USD file
    /// \param[in] _inputFileNameUsd Path where the USD is located
    /// \param[in] _useGazeboPlugins Whether _world should have gazebo plugins
    /// in it (true) or not (false)
    /// \param[out] _world World interface where all USD world data is placed
    /// \return gz::usd::UsdErrors, which is a vector of UsdError objects.
    /// Each UsdErrorincludes an error code and message. An empty vector
    /// indicates no error occurred when parsing the world information of
    /// _inputFileNameUsd
    gz::usd::UsdErrors parseUSDWorld(
      const std::string &_inputFileNameUsd,
      bool _useGazeboPlugins,
      sdf::World &_world);
  }
  }
}
#endif
