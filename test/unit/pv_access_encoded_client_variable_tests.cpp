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

#include <oac-tree/pvxs/pv_access_encoded_client_variable.h>
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

class PvAccessEncodedClientVariableTest : public ::testing::Test
{
protected:
  PvAccessEncodedClientVariableTest();
  ~PvAccessEncodedClientVariableTest();
};

TEST_F(PvAccessEncodedClientVariableTest, VariableRegistration)
{
  auto registry = sup::oac_tree::GlobalVariableRegistry();
  auto names = registry.RegisteredVariableNames();
  ASSERT_TRUE(std::find(names.begin(), names.end(), PvAccessEncodedClientVariable::Type)
              != names.end());
  ASSERT_TRUE(dynamic_cast<PvAccessEncodedClientVariable*>(
                registry.Create(PvAccessEncodedClientVariable::Type).get()));
}

TEST_F(PvAccessEncodedClientVariableTest, Setup)
{
  Workspace ws;
  // channel attribute is mandatory
  {
    PvAccessEncodedClientVariable variable;
    EXPECT_THROW(variable.Setup(ws), VariableSetupException);
    EXPECT_TRUE(variable.AddAttribute("channel", "Not_Relevant"));
    EXPECT_NO_THROW(variable.Setup(ws));
    EXPECT_NO_THROW(variable.Teardown());
  }
}

TEST_F(PvAccessEncodedClientVariableTest, NonExistingChannel)
{
  Workspace ws;
  PvAccessEncodedClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "Does_Not_Exist"));
  EXPECT_NO_THROW(variable.Setup(ws));

  sup::dto::AnyValue value;
  EXPECT_FALSE(variable.GetValue(value));
  EXPECT_TRUE(sup::dto::IsEmptyValue(value));
  value = 5;
  EXPECT_FALSE(variable.SetValue(value));
  EXPECT_NO_THROW(variable.Teardown());
}

//! Server creates a simple scalar value.
//! Check that we can get and set the value from PvAccessEncodedClientVariable.
TEST_F(PvAccessEncodedClientVariableTest, ServerClient)
{
  // server variable
  std::string channel = "PvAccessEncodedClientVariableTest:uint64";
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

  // setting the value
  sup::dto::AnyValue new_val{sup::dto::UnsignedInteger64Type, 1729U};
  EXPECT_TRUE(ws.SetValue("client", new_val));

  // reading from the server
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("server", tmp) && tmp == new_val;
  }));
}

PvAccessEncodedClientVariableTest::PvAccessEncodedClientVariableTest() = default;
PvAccessEncodedClientVariableTest::~PvAccessEncodedClientVariableTest() = default;
