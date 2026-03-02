/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP oac-tree
 *
 * Description   : Unit test code
 *
 * Author        : Gennady Pospelov (IO)
 *
 * Copyright (c) : 2010-2026 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 * SPDX-License-Identifier: MIT
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file LICENSE located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include <oac-tree/pvxs/pv_access_helper.h>

#include <gtest/gtest.h>

using namespace sup::oac_tree;

class PvAccessHelperTest : public ::testing::Test
{
protected:
  PvAccessHelperTest();
  ~PvAccessHelperTest();
};

TEST_F(PvAccessHelperTest, ConvertToEmptyTargetType)
{
  {
    // Scalar remains scalar (although this is not used in PvAccess variables)
    sup::dto::AnyType anytype{};
    sup::dto::AnyValue value{sup::dto::UnsignedInteger64Type, 42U};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, value);
  }
  {
    // Struct remains identical
    sup::dto::AnyType anytype{};
    sup::dto::AnyValue value = {{
      { "mode", {sup::dto::UnsignedInteger16Type, 3U }},
      { "setpoint", {sup::dto::Float64Type, 3.14 }}
    }};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, value);
  }
  {
    // Array remains identical
    sup::dto::AnyType anytype{};
    sup::dto::AnyValue value = sup::dto::ArrayValue({0, 1, 2, 3, 4});
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, value);
  }
}

TEST_F(PvAccessHelperTest, ConvertToScalarTargetType)
{
  {
    // Scalar remains scalar (although this is not used in PvAccess variables)
    sup::dto::AnyType anytype{sup::dto::UnsignedInteger32Type};
    sup::dto::AnyValue value{sup::dto::UnsignedInteger8Type, 42U};
    sup::dto::AnyValue expected{sup::dto::UnsignedInteger32Type, 42U};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Scalar that doesn't allow conversion (too big) will fail and return empty value
    sup::dto::AnyType anytype{sup::dto::UnsignedInteger8Type};
    sup::dto::AnyValue value{sup::dto::UnsignedInteger32Type, 1729U};
    sup::dto::AnyValue expected{};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Struct with 'value' field will try to convert that field to the scalar type
    sup::dto::AnyType anytype{sup::dto::UnsignedInteger32Type};
    sup::dto::AnyValue value = {{
      { "value", {sup::dto::UnsignedInteger8Type, 3U }},
      { "ignored", "this field will not be considered"}
    }};
    sup::dto::AnyValue expected{sup::dto::UnsignedInteger32Type, 3U};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Struct without 'value' field will fail and return an empty value
    sup::dto::AnyType anytype{sup::dto::UnsignedInteger32Type};
    sup::dto::AnyValue value = {{
      { "not_value", {sup::dto::UnsignedInteger8Type, 3U }},
      { "ignored", "this field will not be considered"}
    }};
    sup::dto::AnyValue expected{};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Array will fail to convert to scalar
    sup::dto::AnyType anytype{sup::dto::UnsignedInteger32Type};
    sup::dto::AnyValue value = sup::dto::ArrayValue({0, 1, 2, 3, 4});
    sup::dto::AnyValue expected{};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
}

TEST_F(PvAccessHelperTest, ConvertToStructTargetType)
{
  {
    // Scalar will not convert to struct
    sup::dto::AnyType anytype{{
      { "mode", sup::dto::UnsignedInteger32Type },
      { "description", sup::dto::StringType }
    }};
    sup::dto::AnyValue value{sup::dto::UnsignedInteger8Type, 42U};
    sup::dto::AnyValue expected{};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Struct with same fields will do only scalar conversion of leafs
    sup::dto::AnyType anytype{{
      { "mode", sup::dto::UnsignedInteger32Type },
      { "description", sup::dto::StringType }
    }};
    sup::dto::AnyValue value = {{
      { "mode", {sup::dto::UnsignedInteger8Type, 42U }},
      { "description", "this field does not need conversion"}
    }};
    sup::dto::AnyValue expected = {{
      { "mode", {sup::dto::UnsignedInteger32Type, 42U }},
      { "description", "this field does not need conversion"}
    }};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Struct with superset of fields will just drop those fields in conversion
    sup::dto::AnyType anytype{{
      { "mode", sup::dto::UnsignedInteger32Type },
      { "description", sup::dto::StringType }
    }};
    sup::dto::AnyValue value = {{
      { "mode", {sup::dto::UnsignedInteger8Type, 42U }},
      { "description", "this field does not need conversion"},
      { "not_relevant", true }
    }};
    sup::dto::AnyValue expected = {{
      { "mode", {sup::dto::UnsignedInteger32Type, 42U }},
      { "description", "this field does not need conversion"}
    }};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Struct with subset of fields will not convert and return empty
    sup::dto::AnyType anytype{{
      { "mode", sup::dto::UnsignedInteger32Type },
      { "description", sup::dto::StringType }
    }};
    sup::dto::AnyValue value = {{
      { "mode", {sup::dto::UnsignedInteger8Type, 42U }}
    }};
    sup::dto::AnyValue expected{};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
}

TEST_F(PvAccessHelperTest, ConvertToArrayTargetType)
{
  {
    // Scalar will not convert to array
    sup::dto::AnyType anytype{2, sup::dto::UnsignedInteger16Type};
    sup::dto::AnyValue value{sup::dto::UnsignedInteger8Type, 42U};
    sup::dto::AnyValue expected{};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Struct will not convert to array
    sup::dto::AnyType anytype{2, sup::dto::UnsignedInteger16Type};
    sup::dto::AnyValue value = {{
      { "0", {sup::dto::UnsignedInteger8Type, 42U }},
      { "1", {sup::dto::UnsignedInteger8Type, 42U }}
    }};
    sup::dto::AnyValue expected{};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Array of right size will convert
    sup::dto::AnyType anytype{2, sup::dto::UnsignedInteger16Type};
    sup::dto::AnyValue value = sup::dto::ArrayValue({ {sup::dto::UnsignedInteger8Type, 42U }, 43U});
    sup::dto::AnyValue expected =
      sup::dto::ArrayValue({ {sup::dto::UnsignedInteger16Type, 42U }, 43U});
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Array of larger size will not convert and return empty
    sup::dto::AnyType anytype{2, sup::dto::UnsignedInteger16Type};
    sup::dto::AnyValue value =
      sup::dto::ArrayValue({ {sup::dto::UnsignedInteger8Type, 42U }, 43U, 44U});
    sup::dto::AnyValue expected{};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
  {
    // Array of smaller size will not convert and return empty
    sup::dto::AnyType anytype{3, sup::dto::UnsignedInteger16Type};
    sup::dto::AnyValue value =
      sup::dto::ArrayValue({ {sup::dto::UnsignedInteger8Type, 42U }, 43U});
    sup::dto::AnyValue expected{};
    auto result = pv_access_helper::ConvertToTypedAnyValue(value, anytype);
    EXPECT_EQ(result, expected);
  }
}

PvAccessHelperTest::PvAccessHelperTest() = default;
PvAccessHelperTest::~PvAccessHelperTest() = default;
