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

#ifndef GZ_USD_USD_USD_PARSER_USDSTAGE_HH_
#define GZ_USD_USD_USD_PARSER_USDSTAGE_HH_

#include <set>
#include <string>

#include <ignition/utils/ImplPtr.hh>

#include "sdf/Types.hh"
#include <gz/usd/config.hh>
#include "sdf/system_util.hh"
#include "gz/usd/Export.hh"
#include "gz/usd/UsdError.hh"

namespace gz
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_USD_VERSION_NAMESPACE {
  //
  namespace usd
  {
    /// \brief this class will keep the data of a stage
    /// - UpAxis
    /// - MetersPerUnit
    /// - All USD paths
    class GZ_USD_VISIBLE USDStage
    {
      /// \brief Default constructor
      /// \param[in] _refFileName File name of the stage in the disk
      public: explicit USDStage(const std::string &_refFileName);

      /// \brief Initialize the data structure
      /// \return A vector of Error objects. Each Error includes
      /// an error code and message. An empty vector indicates no error.
      public: gz::usd::UsdErrors Init();

      /// \brief Get stage up axis
      public: const std::string &UpAxis() const;

      /// \brief Get stage meter per unit
      public: double MetersPerUnit() const;

      /// \brief Get USD paths available in the stage
      public: const std::set<std::string> &USDPaths() const;

      /// \brief Private data pointer.
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
  }
}
#endif  // GZ_USD_USD_PARSER_USDSTAGE_HH
