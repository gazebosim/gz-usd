/*
 * Copyright 2022 Open Source Robotics Foundation
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

#include "gz/usd/UsdError.hh"

#include <iostream>

class gz::usd::UsdError::Implementation
{
  /// \brief The error code value.
  public: gz::usd::UsdErrorCode code = gz::usd::UsdErrorCode::NONE;

  /// \brief Description of the error.
  public: std::string message = "";

  /// \brief File path where the error was raised.
  public: std::optional<std::string> filePath = std::nullopt;

  /// \brief Line number in the file path where the error was raised.
  public: std::optional<int> lineNumber = std::nullopt;

  /// \brief SDF error
  public: std::optional<sdf::Error> sdfError = std::nullopt;
};

namespace gz
{
inline namespace GZ_USD_VERSION_NAMESPACE {
//
namespace usd
{
/////////////////////////////////////////////////
UsdError::UsdError()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
UsdError::UsdError(const UsdErrorCode _code, const std::string &_message)
    : dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->code = _code;
  this->dataPtr->message = _message;
}

/////////////////////////////////////////////////
UsdError::UsdError(const UsdErrorCode _code, const std::string &_message,
                   const std::string &_filePath)
    : dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->code = _code;
  this->dataPtr->message = _message;
  this->dataPtr->filePath = _filePath;
}

/////////////////////////////////////////////////
UsdError::UsdError(const UsdErrorCode _code, const std::string &_message,
                   const std::string &_filePath, int _lineNumber)
    : dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->code = _code;
  this->dataPtr->message = _message;
  this->dataPtr->filePath = _filePath;
  this->dataPtr->lineNumber = _lineNumber;
}

/////////////////////////////////////////////////
UsdError::UsdError(const sdf::Error &_sdfError)
    : dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->code = UsdErrorCode::GZ_USD_ERROR;
  this->dataPtr->sdfError = _sdfError;
}

/////////////////////////////////////////////////
UsdErrorCode UsdError::Code() const
{
  return this->dataPtr->code;
}

/////////////////////////////////////////////////
const std::string &UsdError::Message() const
{
  return this->dataPtr->message;
}

/////////////////////////////////////////////////
const std::optional<std::string> &UsdError::FilePath() const
{
  return this->dataPtr->filePath;
}

/////////////////////////////////////////////////
void UsdError::SetFilePath(const std::string &_filePath)
{
  this->dataPtr->filePath = _filePath;
}

/////////////////////////////////////////////////
std::optional<int> UsdError::LineNumber() const
{
  return this->dataPtr->lineNumber;
}

/////////////////////////////////////////////////
void UsdError::SetLineNumber(int _lineNumber)
{
  this->dataPtr->lineNumber = _lineNumber;
}

/////////////////////////////////////////////////
std::optional<sdf::Error> UsdError::SdfError() const
{
  return this->dataPtr->sdfError;
}

/////////////////////////////////////////////////
UsdError::operator bool() const
{
  return this->dataPtr->code != UsdErrorCode::NONE;
}

/////////////////////////////////////////////////
bool UsdError::operator==(const bool _value) const
{
  return ((this->dataPtr->code != UsdErrorCode::NONE) && _value) ||
         ((this->dataPtr->code == UsdErrorCode::NONE) && !_value);
}

/////////////////////////////////////////////////
std::ostream &operator<<(std::ostream &_out,
                         const gz::usd::UsdError &_err)
{
  if (_err.Code() == UsdErrorCode::GZ_USD_ERROR)
  {
    // make sure that an SdfError was wrapped into the UsdError if the error
    // code indicates an SDF error
    if (_err.SdfError())
    {
      _out << _err.SdfError().value();
    }
    else
    {
      _out << "Error code is of type GZ_USD_ERROR, but no sdf::Error "
           << "object has been wrapped into this UsdError object.";
    }

    return _out;
  }

  std::string pathInfo = "";

  if (_err.FilePath().has_value())
    pathInfo += _err.FilePath().value();

  if (_err.LineNumber().has_value())
    pathInfo += ":L" + std::to_string(_err.LineNumber().value());

  if (!pathInfo.empty())
    pathInfo = "[" + pathInfo + "]: ";

  _out << "Error Code "
       << static_cast<std::underlying_type<gz::usd::UsdErrorCode>::type>(
              _err.Code())
       << ": " << pathInfo << "Msg: " << _err.Message();
  return _out;
}
}  // namespace usd
}  // GZ_USD_VERSION_NAMESPACE
}  // namespace gz
