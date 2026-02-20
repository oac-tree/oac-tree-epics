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

#include <oac-tree/pvxs/pv_access_encoded_server_variable.h>

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/variable.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <gtest/gtest.h>
#include <sup/epics-test/unit_test_helper.h>

using namespace sup::oac_tree;

static const std::string UINT16_STRUCT_TYPE =
  R"RAW({"type":"seq-test::uint16-struct-type","attributes":[{"value":{"type":"uint16"}}]})RAW";

static const std::string UINT16_STRUCT_VALUE = R"RAW({"value":42})RAW";

static const std::string UINT16_STRUCT_WRONG_VALUE =
  R"RAW({"value":"A_String!"})RAW";

class PvAccessEncodedServerVariableTest : public ::testing::Test {};

TEST_F(PvAccessEncodedServerVariableTest, VariableRegistration)
{
  auto registry = GlobalVariableRegistry();
  auto names = registry.RegisteredVariableNames();
  ASSERT_TRUE(std::find(names.begin(), names.end(), PvAccessEncodedServerVariable::Type)
              != names.end());
  EXPECT_TRUE(dynamic_cast<PvAccessEncodedServerVariable*>(
                registry.Create(PvAccessEncodedServerVariable::Type).get()));
}

TEST_F(PvAccessEncodedServerVariableTest, Setup)
{
  Workspace ws;
  // channel is mandatory
  {
    PvAccessEncodedServerVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "pvaccess-encoded-server-test::setup"));
    EXPECT_NO_THROW(variable.Setup(ws));
    EXPECT_NO_THROW(variable.Teardown());
  }
  // type attribute must be parsed correctly
  {
    PvAccessEncodedServerVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "pvaccess-encoded-server-test::setup"));
    EXPECT_TRUE(variable.AddAttribute("type", "cannot_be_parsed"));
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
  }
  // value attribute must be parsed correctly
  {
    PvAccessEncodedServerVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "pvaccess-encoded-server-test::setup"));
    EXPECT_TRUE(variable.AddAttribute("type", UINT16_STRUCT_TYPE));
    EXPECT_TRUE(variable.AddAttribute("value", UINT16_STRUCT_WRONG_VALUE));
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
  }
  // Reset variable
  {
    PvAccessEncodedServerVariable variable1;
    EXPECT_NO_THROW(variable1.Reset(ws));

    PvAccessEncodedServerVariable variable2;
    EXPECT_TRUE(variable2.AddAttribute("channel", "pvaccess-encoded-server-test::setup2"));
    EXPECT_TRUE(variable2.AddAttribute("type", UINT16_STRUCT_TYPE));
    EXPECT_TRUE(variable2.AddAttribute("value", UINT16_STRUCT_VALUE));
    EXPECT_NO_THROW(variable2.Setup(ws));
    EXPECT_NO_THROW(variable2.Reset(ws));
    std::string value;
    // Reset has no effect on the value attribute
    variable2.GetAttributeValue("value", value);
    auto reset_does_nothing = ( value == UINT16_STRUCT_VALUE );
    EXPECT_TRUE(reset_does_nothing);
  }
}

TEST_F(PvAccessEncodedServerVariableTest, ScalarSetup)
{
  Workspace ws;
  PvAccessEncodedServerVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "pvaccess-encoded-server-test::scalar-setup"));
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

TEST_F(PvAccessEncodedServerVariableTest, ServerClientTest)
{
  // server variable
  std::string channel = "PvAccessEncodedServerVariableTest:uint64";
  auto server_var = GlobalVariableRegistry().Create("PvAccessEncodedServer");
  ASSERT_TRUE(server_var);
  EXPECT_NO_THROW(server_var->AddAttribute("channel", channel));
  EXPECT_NO_THROW(server_var->AddAttribute("type", R"RAW({"type":"uint64"})RAW"));
  EXPECT_NO_THROW(server_var->AddAttribute("value", R"RAW(42)RAW"));

  // client variable
  auto client_var = GlobalVariableRegistry().Create("PvAccessEncodedClient");
  ASSERT_TRUE(client_var);
  EXPECT_NO_THROW(client_var->AddAttribute("channel", channel));

  // Add variables to workspace
  Workspace ws;
  EXPECT_TRUE(ws.AddVariable("server", std::move(server_var)));
  EXPECT_TRUE(ws.AddVariable("client", std::move(client_var)));
  EXPECT_NO_THROW(ws.Setup());

  // Reading the value from PvAccessEncodedClientVariable
  sup::dto::AnyValue expected_val{sup::dto::UnsignedInteger64Type, 42U};
  EXPECT_TRUE(ws.WaitForVariable("client", 5.0));
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("client", tmp) && tmp == expected_val;
  }));

  // setting the value on the server
  sup::dto::AnyValue new_val{sup::dto::UnsignedInteger64Type, 1729U};
  EXPECT_TRUE(ws.SetValue("server", new_val));

  // reading from the client
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("client", tmp) && tmp == new_val;
  }));
}
