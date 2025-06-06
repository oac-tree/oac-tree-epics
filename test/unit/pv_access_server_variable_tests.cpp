/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP oac-tree
 *
 * Description   : Unit test code
 *
 * Author        : Walter Van Herck (IO)
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

#include <oac-tree/pvxs/pv_access_server_variable.h>

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/sequence_parser.h>
#include <sup/oac-tree/user_interface.h>
#include <sup/oac-tree/variable.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/json_type_parser.h>

#include <gtest/gtest.h>
#include <sup/epics-test/unit_test_helper.h>
#include "unit_test_helper.h"

using namespace sup::oac_tree;

static const std::string UINT16_STRUCT_TYPE =
  R"RAW({"type":"seq-test::uint16-struct-type","attributes":[{"value":{"type":"uint16"}}]})RAW";

static const std::string UINT16_STRUCT_VALUE = R"RAW({"value":42})RAW";

static const std::string UINT16TYPE =
  R"RAW({"type":"uint16"})RAW";

static const std::string UINT16_STRUCT_WRONG_VALUE =
  R"RAW({"value":"A_String!"})RAW";

class PvAccessServerVariableTest : public ::testing::Test
{
protected:
  PvAccessServerVariableTest();
  ~PvAccessServerVariableTest();
};

TEST_F(PvAccessServerVariableTest, VariableRegistration)
{
  auto registry = GlobalVariableRegistry();
  auto names = registry.RegisteredVariableNames();
  ASSERT_TRUE(std::find(names.begin(), names.end(), PvAccessServerVariable::Type) != names.end());
  EXPECT_TRUE(dynamic_cast<PvAccessServerVariable*>(registry.Create(PvAccessServerVariable::Type).get()));
}

TEST_F(PvAccessServerVariableTest, Setup)
{
  Workspace ws;
  // channel and type attribute is mandatory
  {
    PvAccessServerVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "pvaccess-server-var-test::setup"));
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("type", UINT16_STRUCT_TYPE));
    EXPECT_NO_THROW(variable.Setup(ws));
    EXPECT_NO_THROW(variable.Teardown());
  }
  // type attribute must be parsed correctly
  {
    PvAccessServerVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "pvaccess-server-var-test::setup"));
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("type", "cannot_be_parsed"));
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
  }
  // value attribute must be parsed correctly
  {
    PvAccessServerVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "pvaccess-server-var-test::setup"));
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("type", UINT16_STRUCT_TYPE));
    EXPECT_TRUE(variable.AddAttribute("value", UINT16_STRUCT_WRONG_VALUE));
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
  }
  // Reset variable
  {
    PvAccessServerVariable variable1;
    EXPECT_NO_THROW(variable1.Reset(ws));

    PvAccessServerVariable variable2;
    EXPECT_TRUE(variable2.AddAttribute("channel", "pvaccess-server-var-test::setup2"));
    EXPECT_TRUE(variable2.AddAttribute("type", UINT16_STRUCT_TYPE));
    EXPECT_TRUE(variable2.AddAttribute("value", UINT16_STRUCT_VALUE));
    EXPECT_NO_THROW(variable2.Setup(ws));
    EXPECT_NO_THROW(variable2.Reset(ws));
    std::string value;
    // Reset has no effect on the value attribute
    variable2.GetAttributeValue("value", value);
    auto reset_does_nothing = value == UINT16_STRUCT_VALUE;

    EXPECT_TRUE(reset_does_nothing);
  }
}

TEST_F(PvAccessServerVariableTest, ScalarSetup)
{
  Workspace ws;
  PvAccessServerVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "pvaccess-server-var-test::scalar-setup"));
  EXPECT_NO_THROW(variable.AddAttribute("type", R"RAW({"type":"uint64"})RAW"));
  // Add callback:
  bool val_ok = false;
  auto cb = [&val_ok](const sup::dto::AnyValue& val, bool connected) {
    if (connected && val.GetType() == sup::dto::UnsignedInteger64Type)
    {
      val_ok = true;
    }
  };
  EXPECT_NO_THROW(variable.SetNotifyCallback(cb));
  EXPECT_NO_THROW(variable.Setup(ws));
  EXPECT_TRUE(val_ok);

  sup::dto::AnyValue value;
  EXPECT_TRUE(variable.GetValue(value));
  EXPECT_EQ(value.GetType(), sup::dto::UnsignedInteger64Type);
  EXPECT_TRUE(variable.IsAvailable());
}

TEST_F(PvAccessServerVariableTest, MultipleScalarSetup)
{
  DefaultUserInterface ui;

  std::string procedure_body{
      R"RAW(
  <ForceSuccess isRoot="true">
    <Equals leftVar="True" rightVar="num1"/>
  </ForceSuccess>
  <Workspace>
    <PvAccessServer channel="test::var1" name="num1" type='{"type":"bool"}' value="true"/>
    <PvAccessServer channel="test::var2" name="num2" type='{"type":"uint64"}' value="0"/>
    <Local name="True" type='{"type":"bool"}' value="true"/>
  </Workspace>
)RAW"};

  // read variable when server is online
  const auto procedure_string_true = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string_true);
  EXPECT_TRUE(unit_test_helper::TryAndExecuteNoReset(proc, ui, ExecutionStatus::SUCCESS));
}

//! Server creates a structure with the value.
//! Check that we can get and set the value from PvAccessClientVariable.

TEST_F(PvAccessServerVariableTest, ServerClientTest)
{
  const std::string kDataType =
      R"({"type":"testtype","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]})";
  const char* channel = "PvAccessServerVariableTest:FloatStruct";
  sup::dto::JSONAnyTypeParser type_parser;
  EXPECT_TRUE(type_parser.ParseString(kDataType));
  auto pv_type = type_parser.MoveAnyType();
  sup::dto::AnyValue pv_val{pv_type};

  Workspace ws;
  // creating the server variable
  auto server_var = GlobalVariableRegistry().Create("PvAccessServer");
  EXPECT_NO_THROW(server_var->AddAttribute("channel", channel));
  EXPECT_NO_THROW(server_var->AddAttribute("type", kDataType));
  EXPECT_TRUE(ws.AddVariable("server_var", std::move(server_var)));

  // Creating oac-tree's PvAccessClientVariable
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_NO_THROW(variable->AddAttribute("type", kDataType));
  EXPECT_TRUE(ws.AddVariable("client_var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  // setting the value through the server
  pv_val["value"] = 42.1f;
  EXPECT_TRUE(ws.SetValue("server_var", pv_val));

  // Reading the value from PvAccessClientVariable
  EXPECT_TRUE(ws.WaitForVariable("client_var", 5.0));
  sup::dto::AnyValue var_val;
  EXPECT_TRUE(ws.GetValue("client_var", var_val));
  ASSERT_TRUE(var_val.HasField("value"));
  EXPECT_EQ(var_val["value"], 42.1f);

  // setting the value
  auto new_value = 142.0f;
  EXPECT_TRUE(ws.SetValue("client_var.value", new_value));

  // reading from the server
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ws, new_value]{
    sup::dto::AnyValue server_val;
    return ws.GetValue("server_var", server_val) &&
           server_val.HasField("value") &&
           server_val["value"] == new_value;
  }));
}

PvAccessServerVariableTest::PvAccessServerVariableTest() = default;
PvAccessServerVariableTest::~PvAccessServerVariableTest() = default;
