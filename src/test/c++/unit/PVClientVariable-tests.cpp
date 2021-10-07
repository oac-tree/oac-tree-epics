/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP Sequencer
 *
 * Description   : Unit test code
 *
 * Author        : Gennady Pospelov (IO)
 *
 * Copyright (c) : 2010-2020 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include "PVClientVariable.h"

#include <VariableRegistry.h>
#include <common/PVAccessServer.h>
#include <gtest/gtest.h>

#include <algorithm>

using sup::sequencer::PVClientVariable;

namespace
{
const size_t kSececond = 1e9;
}

class PVClientVariableTest : public ::testing::Test
{
};

TEST_F(PVClientVariableTest, VariableRegistration)
{
  auto registry = sup::sequencer::GlobalVariableRegistry();
  auto names = registry.RegisteredVariableNames();
  ASSERT_TRUE(std::find(names.begin(), names.end(), PVClientVariable::Type) != names.end());
  ASSERT_TRUE(dynamic_cast<PVClientVariable*>(registry.Create(PVClientVariable::Type).get()));
}

TEST_F(PVClientVariableTest, InvalidSetup)
{
  PVClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PVClientVariableTest:INTEGER"));
  // should throw because of type parsing error
  EXPECT_THROW(variable.AddAttribute("datatype", "invalid-type"), std::runtime_error);
}

TEST_F(PVClientVariableTest, ValidSetup)
{
  PVClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PVClientVariableTest:INTEGER"));
  EXPECT_NO_THROW(variable.AddAttribute("datatype", R"RAW({"type":"uint64"})RAW"));
}

TEST_F(PVClientVariableTest, GetNonExistingValue)
{
  PVClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PVClientVariableTest:INTEGER"));
  EXPECT_NO_THROW(variable.AddAttribute("datatype", R"RAW({"type":"uint64"})RAW"));

  ccs::types::AnyValue value;
  // will throw while checking connecting to the variable
  EXPECT_THROW(variable.GetValue(value), std::runtime_error);
}

TEST_F(PVClientVariableTest, GetValue)
{
  const std::string channel("PVClientVariableTest:INTEGER");
  ::ccs::base::PVAccessServer server;

  // creating the variable
  server.AddVariable(channel.c_str(), ccs::types::AnyputVariable, ccs::types::UnsignedInteger32);
  server.Launch();
  ccs::HelperTools::SleepFor(0.1 * kSececond);

  {
    // Attempt to rely on server.SetVariable(channel, any_value)  - Doesn't work
    // Tests for it are absent on PVAccessServer-test.cpp

    // setting the value throught the server
    // ::ccs::types::uint32 value = 42;
    // ::ccs::types::AnyValue any_value(::ccs::types::UnsignedInteger32);
    // any_value = value;
    // ASSERT_TRUE(server.SetVariable(channel.c_str(), any_value));
    // ccs::HelperTools::SleepFor(0.1*kSececond);

    // reading the value through the server
    // ccs::types::AnyValue any_value2;
    // server.GetVariable(channel.c_str(), any_value2);
    // EXPECT_EQ(any_value, any_value2);
  }

  // setting the value throught the server
  ccs::types::uint32 value = 42;
  ASSERT_TRUE(
      ccs::HelperTools::SetAttributeValue(server.GetVariable(channel.c_str()), "value", value));

  // reading the value through the server
  ccs::types::uint32 value2{0};
  ASSERT_TRUE(
      ccs::HelperTools::GetAttributeValue(server.GetVariable(channel.c_str()), "value", value2));
  EXPECT_EQ(value, value2);

  // PVClientVariable variable;
  // EXPECT_NO_THROW(variable.AddAttribute("channel", channel));
  // EXPECT_NO_THROW(variable.AddAttribute("datatype", R"RAW({"type":"uint32"})RAW"));  
  // EXPECT_TRUE(variable.GetValue(any_value2));
}
