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
#include <optional>
#include <gz/usd/config.hh>
#include "gz/usd/UsdError.hh"

/////////////////////////////////////////////////
TEST(Error, DefaultConstruction)
{
  gz::usd::UsdError error;
  EXPECT_FALSE(error);
  EXPECT_EQ(error.Code(), gz::usd::UsdErrorCode::NONE);
  EXPECT_TRUE(error.Message().empty());
  EXPECT_FALSE(error.FilePath().has_value());
  EXPECT_FALSE(error.LineNumber().has_value());
  EXPECT_FALSE(error.SdfError().has_value());

  if (error)
  {
    FAIL();
  }
  error.SetFilePath("/usd/world");
  ASSERT_TRUE(error.FilePath().has_value());
  EXPECT_EQ("/usd/world", error.FilePath());

  error.SetLineNumber(5);
  ASSERT_TRUE(error.LineNumber().has_value());
  EXPECT_EQ(5, error.LineNumber());
}

/////////////////////////////////////////////////
TEST(Error, ValueConstructionWithoutFile)
{
  gz::usd::UsdError error(
    gz::usd::UsdErrorCode::INVALID_PRIM_PATH,
    "Invalid Prim path");
  EXPECT_TRUE(error);
  EXPECT_EQ(error.Code(), gz::usd::UsdErrorCode::INVALID_PRIM_PATH);
  EXPECT_EQ(error.Message(), "Invalid Prim path");
  EXPECT_FALSE(error.FilePath().has_value());
  EXPECT_FALSE(error.LineNumber().has_value());
  EXPECT_FALSE(error.SdfError().has_value());

  if (!error)
    FAIL();
}

/////////////////////////////////////////////////
TEST(Error, ValueConstructionWithFile)
{
  const std::string emptyFilePath = "Empty/file/path";
  gz::usd::UsdError error(
    gz::usd::UsdErrorCode::INVALID_PRIM_PATH,
    "Invalid Prim path",
    emptyFilePath);
  EXPECT_TRUE(error);
  EXPECT_EQ(error.Code(), gz::usd::UsdErrorCode::INVALID_PRIM_PATH);
  EXPECT_EQ(error.Message(), "Invalid Prim path");
  ASSERT_TRUE(error.FilePath().has_value());
  EXPECT_EQ(error.FilePath().value(), emptyFilePath);
  EXPECT_FALSE(error.LineNumber().has_value());
  EXPECT_FALSE(error.SdfError().has_value());

  if (!error)
    FAIL();
}

/////////////////////////////////////////////////
TEST(Error, ValueConstructionWithLineNumber)
{
  const std::string emptyFilePath = "Empty/file/path";
  const int lineNumber = 10;
  gz::usd::UsdError error(
    gz::usd::UsdErrorCode::INVALID_PRIM_PATH,
    "Invalid Prim path",
    emptyFilePath,
    lineNumber);
  EXPECT_TRUE(error);
  EXPECT_EQ(error.Code(), gz::usd::UsdErrorCode::INVALID_PRIM_PATH);
  EXPECT_EQ(error.Message(), "Invalid Prim path");
  ASSERT_TRUE(error.FilePath().has_value());
  EXPECT_EQ(error.FilePath().value(), emptyFilePath);
  ASSERT_TRUE(error.LineNumber().has_value());
  EXPECT_EQ(error.LineNumber().value(), lineNumber);
  EXPECT_FALSE(error.SdfError().has_value());

  if (!error)
    FAIL();
}

/////////////////////////////////////////////////
TEST(Error, sdfError)
{
  const std::string emptyFilePath = "Empty/file/sdfpath";

  const std::string sdfErrorMessage = "Unable to read a file";
  sdf::Error errorSdf(
    sdf::ErrorCode::FILE_READ,
    sdfErrorMessage,
    emptyFilePath);

  gz::usd::UsdError error(errorSdf);
  EXPECT_TRUE(error);
  EXPECT_EQ(error.Code(), gz::usd::UsdErrorCode::GZ_USD_ERROR);
  EXPECT_EQ(error.Message(), "");
  EXPECT_FALSE(error.FilePath().has_value());
  EXPECT_FALSE(error.LineNumber().has_value());

  auto errorSdf2 = error.SdfError();
  ASSERT_TRUE(errorSdf2.has_value());
  EXPECT_EQ(errorSdf2.value().Code(), sdf::ErrorCode::FILE_READ);
  EXPECT_EQ(errorSdf2.value().Message(), sdfErrorMessage);
  EXPECT_EQ(errorSdf2.value().FilePath().value(), emptyFilePath);
  EXPECT_FALSE(errorSdf2.value().XmlPath().has_value());

  if (!error)
    FAIL();

  if (!errorSdf2)
    FAIL();
}
