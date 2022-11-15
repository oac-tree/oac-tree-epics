/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP Sequencer
 *
 * Description   : Unit test code
 *
 * Author        : Walter Van Herck (IO)
 *
 * Copyright (c) : 2010-2022 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include "null_user_interface.h"
#include "unit_test_helper.h"

#include <pvxs/pv_access_monitor_variable.h>

#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/json_type_parser.h>
#include <sup/epics/pv_access_server.h>

#include <gtest/gtest.h>

#include <algorithm>

using namespace sup::sequencer;

class PvAccessMonitorVariableTest : public ::testing::Test
{
protected:
  PvAccessMonitorVariableTest();
  ~PvAccessMonitorVariableTest();
};

TEST_F(PvAccessMonitorVariableTest, VariableRegistration)
{
  auto registry = sup::sequencer::GlobalVariableRegistry();
  auto names = registry.RegisteredVariableNames();
  ASSERT_TRUE(std::find(names.begin(), names.end(), PvAccessMonitorVariable::Type) != names.end());
  ASSERT_TRUE(dynamic_cast<PvAccessMonitorVariable*>(registry.Create(PvAccessMonitorVariable::Type).get()));
}

TEST_F(PvAccessMonitorVariableTest, InvalidSetup)
{
  PvAccessMonitorVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PvAccessMonitorVariableTest:INTEGER"));
  EXPECT_NO_THROW(variable.AddAttribute("type", "invalid-type"));
  EXPECT_NO_THROW(variable.Setup());
}

TEST_F(PvAccessMonitorVariableTest, GetValue_error)
{
  const std::string kDataType =
      R"({"type":"testtype","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]})";
  const char* channel = "PvAccessMonitorVariableTest:FloatStruct";
  sup::dto::JSONAnyTypeParser type_parser;
  EXPECT_TRUE(type_parser.ParseString(kDataType));
  auto pv_type = type_parser.MoveAnyType();
  sup::dto::AnyValue pv_val{pv_type};

  sup::epics::PvAccessServer server;

  // creating the variable
  server.AddVariable(channel, pv_val);
  server.Start();

  // setting the value through the server
  pv_val["value"] = 42.1f;
  EXPECT_TRUE(server.SetValue(channel, pv_val));

  // Creating sequencer's PvAccessMonitorVariable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("PvAccessMonitor");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  // Reading the value from PvAccessMonitorVariable
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  sup::dto::AnyValue var_val;
  EXPECT_TRUE(ws.GetValue("var", var_val));
  ASSERT_TRUE(var_val.HasField("value"));
  EXPECT_EQ(var_val["value"], 42.1f);

  // setting the value fails since it is read-only
  auto new_value = 142.0f;
  EXPECT_FALSE(ws.SetValue("var.value", new_value));
}

//! Server creates a structure with a value.
//! Check that we can get the value from PvAccessMonitorVariable with scalar type.

TEST_F(PvAccessMonitorVariableTest, PvAccessScalarClientTest)
{
  const std::string kDataType =
      R"({"type":"testtype","attributes":[{"value":{"type":"float32"}}]})";
  const char* channel = "PvAccessMonitorVariableTest:ScalarFloat";
  sup::dto::JSONAnyTypeParser type_parser;
  EXPECT_TRUE(type_parser.ParseString(kDataType));
  auto pv_type = type_parser.MoveAnyType();
  sup::dto::AnyValue pv_val{pv_type};

  sup::epics::PvAccessServer server;

  // creating the variable
  server.AddVariable(channel, pv_val);
  server.Start();

  // setting the value through the server
  pv_val["value"] = 42.1f;
  EXPECT_TRUE(server.SetValue(channel, pv_val));

  // Creating sequencer's PvAccessMonitorVariable
  Workspace ws;
  const std::string kFloatType = R"({"type":"float32"})";
  auto variable = GlobalVariableRegistry().Create("PvAccessMonitor");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_NO_THROW(variable->AddAttribute("type", kFloatType));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  // Reading the value from PvAccessMonitorVariable
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  sup::dto::AnyValue var_val;
  EXPECT_TRUE(ws.GetValue("var", var_val));
  EXPECT_EQ(var_val.As<float>(), 42.1f);

  // setting the value
  auto new_value = 142.0f;
  EXPECT_FALSE(ws.SetValue("var", new_value));
}

PvAccessMonitorVariableTest::PvAccessMonitorVariableTest() = default;
PvAccessMonitorVariableTest::~PvAccessMonitorVariableTest() = default;
