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

#include "test_user_interface.h"
#include "unit_test_helper.h"

#include <pvxs/pv_access_client_variable.h>

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/json_type_parser.h>
#include <sup/epics/pv_access_server.h>

#include <gtest/gtest.h>

#include <algorithm>

using namespace sup::sequencer;

class PvAccessClientVariableTest : public ::testing::Test
{
protected:
  PvAccessClientVariableTest();
  ~PvAccessClientVariableTest();
};

TEST_F(PvAccessClientVariableTest, VariableRegistration)
{
  auto registry = sup::sequencer::GlobalVariableRegistry();
  auto names = registry.RegisteredVariableNames();
  ASSERT_TRUE(std::find(names.begin(), names.end(), PvAccessClientVariable::Type) != names.end());
  ASSERT_TRUE(dynamic_cast<PvAccessClientVariable*>(registry.Create(PvAccessClientVariable::Type).get()));
}

TEST_F(PvAccessClientVariableTest, Setup)
{
  // channel attribute is mandatory
  {
    PvAccessClientVariable variable;
    EXPECT_THROW(variable.Setup(), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "Not_Relevant"));
    EXPECT_NO_THROW(variable.Setup());
    EXPECT_NO_THROW(variable.Reset());
  }
  // type attribute should be non empty
  {
    PvAccessClientVariable variable;
    EXPECT_THROW(variable.Setup(), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "Not_Relevant"));
    EXPECT_TRUE(variable.AddAttribute("type", ""));
    EXPECT_THROW(variable.Setup(), VariableSetupException);
  }
  // type attribute should be a json representation of an AnyType
  {
    PvAccessClientVariable variable;
    EXPECT_THROW(variable.Setup(), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "Not_Relevant"));
    EXPECT_TRUE(variable.AddAttribute("type", "cannot_be_parsed"));
    EXPECT_THROW(variable.Setup(), VariableSetupException);
  }
  // valid type attribute
  {
    PvAccessClientVariable variable;
    EXPECT_THROW(variable.Setup(), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "Not_Relevant"));
    EXPECT_TRUE(variable.AddAttribute("type", R"RAW({"type":"uint64"})RAW"));
    EXPECT_NO_THROW(variable.Setup());
    EXPECT_NO_THROW(variable.Reset());
  }
}

TEST_F(PvAccessClientVariableTest, NonExistingChannel)
{
  PvAccessClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "Does_Not_Exist"));
  EXPECT_NO_THROW(variable.AddAttribute("type", R"RAW({"type":"uint64"})RAW"));
  EXPECT_NO_THROW(variable.Setup());

  sup::dto::AnyValue value;
  EXPECT_FALSE(variable.GetValue(value));
  EXPECT_TRUE(sup::dto::IsEmptyValue(value));
  value = 5;
  EXPECT_FALSE(variable.SetValue(value));
  EXPECT_NO_THROW(variable.Reset());
}

//! Server creates a structure with the value.
//! Check that we can get and set the value from PvAccessClientVariable.
TEST_F(PvAccessClientVariableTest, ServerClient)
{
  const std::string kDataType =
      R"({"type":"testtype","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]})";
  std::string channel = "PvAccessClientVariableTest:FloatStruct";
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

  // Creating sequencer's PvAccessClientVariable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_NO_THROW(variable->AddAttribute("type", kDataType));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  // Reading the value from PvAccessClientVariable
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.HasField("value") && tmp["value"] == 42.1f;
  }));

  // setting the value
  auto new_value = 142.0f;
  EXPECT_TRUE(ws.SetValue("var.value", new_value));

  // reading from the server
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&server, channel, new_value]{
    auto server_val = server.GetValue(channel);
    return server_val.HasField("value") && server_val["value"] == new_value;
  }));
}

TEST_F(PvAccessClientVariableTest, ServerClientNoType)
{
  const std::string kDataType =
      R"({"type":"testtype","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]})";
  std::string channel = "PvAccessClientVariableTest:NoType";
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

  // Creating sequencer's PvAccessClientVariable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  // Reading the value from PvAccessClientVariable
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.HasField("value") && tmp["value"] == 42.1f;
  }));

  // setting the value
  auto new_value = 142.0f;
  EXPECT_TRUE(ws.SetValue("var.value", new_value));

  // reading from the server
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&server, channel, new_value]{
    auto server_val = server.GetValue(channel);
    return server_val.HasField("value") && server_val["value"] == new_value;
  }));
}

//! Server creates a structure with a value.
//! Check that we can get and set the value from PvAccessClientVariable with scalar type.
TEST_F(PvAccessClientVariableTest, ScalarClient)
{
  const std::string kDataType =
      R"({"type":"testtype","attributes":[{"value":{"type":"float32"}}]})";
  const char* channel = "PvAccessClientVariableTest:ScalarFloat";
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

  // Creating sequencer's PvAccessClientVariable
  Workspace ws;
  const std::string kFloatType = R"({"type":"float32"})";
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_NO_THROW(variable->AddAttribute("type", kFloatType));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  // Reading the value from PvAccessClientVariable
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.GetType() == sup::dto::Float32Type &&
           tmp.As<float>() == 42.1f;
  }));

  // setting the value
  auto new_value = 142.0f;
  EXPECT_TRUE(ws.SetValue("var", new_value));

  // reading from the server
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&server, channel, new_value]{
    auto server_val = server.GetValue(channel);
    return server_val.HasField("value") && server_val["value"] == new_value;
  }));
}

PvAccessClientVariableTest::PvAccessClientVariableTest() = default;
PvAccessClientVariableTest::~PvAccessClientVariableTest() = default;
