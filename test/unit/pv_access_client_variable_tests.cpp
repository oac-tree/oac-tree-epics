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
 * Copyright (c) : 2010-2025 ITER Organization,
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

#include <oac-tree/pvxs/pv_access_client_variable.h>
#include "unit_test_helper.h"

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/sequence_parser.h>
#include <sup/oac-tree/user_interface.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/json_type_parser.h>
#include <sup/epics/pv_access_server.h>

#include <gtest/gtest.h>
#include <sup/epics-test/unit_test_helper.h>

#include <algorithm>

using namespace sup::oac_tree;

class PvAccessClientVariableTest : public ::testing::Test
{
protected:
  PvAccessClientVariableTest();
  ~PvAccessClientVariableTest();
};

TEST_F(PvAccessClientVariableTest, VariableRegistration)
{
  auto registry = sup::oac_tree::GlobalVariableRegistry();
  auto names = registry.RegisteredVariableNames();
  ASSERT_TRUE(std::find(names.begin(), names.end(), PvAccessClientVariable::Type) != names.end());
  ASSERT_TRUE(dynamic_cast<PvAccessClientVariable*>(registry.Create(PvAccessClientVariable::Type).get()));
}

TEST_F(PvAccessClientVariableTest, Setup)
{
  Workspace ws;
  // channel attribute is mandatory
  {
    PvAccessClientVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "Not_Relevant"));
    EXPECT_NO_THROW(variable.Setup(ws));
    EXPECT_NO_THROW(variable.Teardown());
  }
  // type attribute should be non empty
  {
    PvAccessClientVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "Not_Relevant"));
    EXPECT_TRUE(variable.AddAttribute("type", ""));
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
  }
  // type attribute should be a json representation of an AnyType
  {
    PvAccessClientVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "Not_Relevant"));
    EXPECT_TRUE(variable.AddAttribute("type", "cannot_be_parsed"));
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
  }
  // valid type attribute
  {
    PvAccessClientVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "Not_Relevant"));
    EXPECT_TRUE(variable.AddAttribute("type", R"RAW({"type":"uint64"})RAW"));
    EXPECT_NO_THROW(variable.Setup(ws));
    EXPECT_NO_THROW(variable.Teardown());
  }
}

TEST_F(PvAccessClientVariableTest, NonExistingChannel)
{
  Workspace ws;
  PvAccessClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "Does_Not_Exist"));
  EXPECT_NO_THROW(variable.AddAttribute("type", R"RAW({"type":"uint64"})RAW"));
  EXPECT_NO_THROW(variable.Setup(ws));

  sup::dto::AnyValue value;
  EXPECT_FALSE(variable.GetValue(value));
  EXPECT_TRUE(sup::dto::IsEmptyValue(value));
  value = 5;
  EXPECT_FALSE(variable.SetValue(value));
  EXPECT_NO_THROW(variable.Teardown());
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

  // Creating oac-tree's PvAccessClientVariable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_NO_THROW(variable->AddAttribute("type", kDataType));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  // Reading the value from PvAccessClientVariable
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.HasField("value") && tmp["value"] == 42.1f;
  }));

  // setting the value
  auto new_value = 142.0f;
  EXPECT_TRUE(ws.SetValue("var.value", new_value));

  // reading from the server
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&server, channel, new_value]{
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

  // Creating oac-tree's PvAccessClientVariable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  // Reading the value from PvAccessClientVariable
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.HasField("value") && tmp["value"] == 42.1f;
  }));

  // setting the value
  auto new_value = 142.0f;
  EXPECT_TRUE(ws.SetValue("var.value", new_value));

  // reading from the server
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&server, channel, new_value]{
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

  // Creating oac-tree's PvAccessClientVariable
  Workspace ws;
  const std::string kFloatType = R"({"type":"float32"})";
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_NO_THROW(variable->AddAttribute("type", kFloatType));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  // Reading the value from PvAccessClientVariable
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.GetType() == sup::dto::Float32Type &&
           tmp.As<float>() == 42.1f;
  }));

  // setting the value
  auto new_value = 142.0f;
  EXPECT_TRUE(ws.SetValue("var", new_value));

  // reading from the server
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&server, channel, new_value]{
    auto server_val = server.GetValue(channel);
    return server_val.HasField("value") && server_val["value"] == new_value;
  }));
}

TEST_F(PvAccessClientVariableTest, WriteIndexFieldAsBool)
{
  DefaultUserInterface ui;
  std::string procedure_body{
      R"RAW(
    <RegisterType jsontype='{"type":"enum_t","attributes":[{"index":{"type":"int32"}},{"choices":{"type":"","element":{"type":"string"},"multiplicity":2}}]}'/>
    <RegisterType jsontype='{"type":"NTEnum","attributes":[{"value":{"type":"enum_t"}}]}'/>
    <Sequence>
        <WaitForVariables varType="PvAccessClient" timeout="5.0"/>
        <Copy inputVar="b_on" outputVar="enum_local.value.index"/>
        <Copy inputVar="b_on" outputVar="enum_client.value.index"/>
    </Sequence>
    <Workspace>
        <PvAccessServer name="enum_server" channel="seq::test::enum_test" type='{"type":"NTEnum"}'
               value='{"value":{"index":0,"choices":["OFF","ON"]}}'/>
        <PvAccessClient name="enum_client" channel="seq::test::enum_test"/>
        <Local name="enum_local" type='{"type":"NTEnum"}'
               value='{"value":{"index":0,"choices":["OFF","ON"]}}'/>
        <Local name="b_on" type='{"type":"bool"}' value='true'/>
    </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  ASSERT_TRUE(proc);
  EXPECT_TRUE(unit_test_helper::TryAndExecuteNoReset(proc, ui, ExecutionStatus::SUCCESS));
  auto choices = sup::dto::ArrayValue({ "OFF", "ON" });
  sup::dto::AnyValue expected_enum = {{
      { "index", { sup::dto::SignedInteger32Type, 1 } },
      { "choices", choices }
    }, "enum_t" };
  sup::dto::AnyValue expected{{
    { "value", expected_enum }
  }, "NTEnum"};
  Workspace &ws = proc->GetWorkspace();
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ws, expected]{
    sup::dto::AnyValue read_back;
    (void)ws.GetValue("enum_client", read_back);
    return read_back == expected;
  }));
}

PvAccessClientVariableTest::PvAccessClientVariableTest() = default;
PvAccessClientVariableTest::~PvAccessClientVariableTest() = default;
