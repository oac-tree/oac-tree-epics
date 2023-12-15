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

#include <sequencer/pvxs/pv_access_server_variable.h>

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/variable.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/json_type_parser.h>

#include <gtest/gtest.h>
#include <sup/epics-test/unit_test_helper.h>

using namespace sup::sequencer;

static const std::string UINT16_STRUCT_TYPE =
  R"RAW({"type":"seq-test::uint16-struct-type","attributes":[{"value":{"type":"uint16"}}]})RAW";

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
    EXPECT_NO_THROW(variable.Reset());
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
}

TEST_F(PvAccessServerVariableTest, ScalarSetup)
{
  Workspace ws;
  PvAccessServerVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "pvaccess-server-var-test::scalar-setup"));
  EXPECT_NO_THROW(variable.AddAttribute("type", R"RAW({"type":"uint64"})RAW"));
  EXPECT_NO_THROW(variable.Setup(ws));

  sup::dto::AnyValue value;
  EXPECT_TRUE(variable.GetValue(value));
  EXPECT_EQ(value.GetType(), sup::dto::UnsignedInteger64Type);
  EXPECT_TRUE(variable.IsAvailable());
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

  // Creating sequencer's PvAccessClientVariable
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
