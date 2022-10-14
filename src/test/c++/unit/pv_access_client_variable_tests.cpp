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

#include "null_user_interface.h"
#include "unit_test_helper.h"

#include <pvxs/pv_access_client_variable.h>

#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/json_type_parser.h>
#include <sup/epics/pv_access_server.h>
#include <sup/epics/pv_access_context_utils.h>

#include <gtest/gtest.h>

#include <algorithm>

namespace
{
const size_t kSecond = 1e9;

std::string CreateProcedureString(const std::string& body)
{
  static const std::string header{
      R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">)RAW"};

  static const std::string footer{R"RAW(</Procedure>)RAW"};

  return header + body + footer;
}
}  // namespace

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

TEST_F(PvAccessClientVariableTest, InvalidSetup)
{
  PvAccessClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PvAccessClientVariableTest:INTEGER"));
  EXPECT_NO_THROW(variable.AddAttribute("type", "invalid-type"));
  // should throw because of type parsing error
  EXPECT_NO_THROW(variable.Setup());
}

TEST_F(PvAccessClientVariableTest, ValidSetup)
{
  PvAccessClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PvAccessClientVariableTest:INTEGER"));
  EXPECT_NO_THROW(variable.AddAttribute("datatype", R"RAW({"type":"uint64"})RAW"));
  EXPECT_NO_THROW(variable.Setup());
}

TEST_F(PvAccessClientVariableTest, GetNonExistingValue)
{
  PvAccessClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PvAccessClientVariableTest:INTEGER"));
  EXPECT_NO_THROW(variable.AddAttribute("datatype", R"RAW({"type":"uint64"})RAW"));
  EXPECT_NO_THROW(variable.Setup());

  sup::dto::AnyValue value;
  EXPECT_FALSE(variable.GetValue(value));
}

//! Server creates a structure with the value.
//! Check that we can get and set the value from PvAccessClientVariable.

// TEST_F(PvAccessClientVariableTest, PVAccessServerClientTest)
// {
//   const std::string kDataType =
//       R"({"type":"testtype","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]})";
//   const char* channel = "PvAccessClientVariableTest:INTEGER";
//   sup::dto::JSONAnyTypeParser type_parser;
//   EXPECT_TRUE(type_parser.ParseString(kDataType));
//   auto pv_type = type_parser.MoveAnyType();
//   sup::dto::AnyValue pv_val{pv_type};

//   sup::epics::PVAccessServer server(sup::epics::CreateIsolatedServer());

//   // creating the variable
//   server.AddVariable(channel, pv_val);
//   server.Start();

//   // setting the value via PVAccessServer::GetVariable
//   pv_val["value"] = 42.1f;
//   EXPECT_TRUE(server.SetValue(channel, pv_val));  // without this call EXPECT below are failing

//   // Creating sequencer's PvAccessClientVariable
//   Workspace ws;
//   auto variable = GlobalVariableRegistry().Create("PvAccessClient");
//   EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
//   EXPECT_NO_THROW(variable->AddAttribute("type", kDataType));
//   EXPECT_TRUE(ws.AddVariable("var", variable.release()));
//   EXPECT_NO_THROW(ws.Setup());

//   // Reading the value from PvAccessClientVariable
//   EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
//   sup::dto::AnyValue var_val;
//   EXPECT_TRUE(ws.GetValue("var", var_val));
//   EXPECT_TRUE(var_val.HasField("value"));
//   EXPECT_EQ(var_val["value"], 42.1f);

//   // setting the value
//   auto new_value = 142.0f;
//   EXPECT_TRUE(ws.SetValue("var.value", new_value));

//   // reading from the server
//   EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&server, channel, new_value]{
//     auto server_val = server.GetValue(channel);
//     return server_val.HasField("value") && server_val["value"] == new_value;
//   }));
// }

//! Structure with value is created on server.
//! Procedure creates PvAccessClientVariable and set new value to it.
//! We expect that the value will be propagated to the server side.

// TEST_F(PvAccessClientVariableTest, ExecuteProcedure)
// {
//   // preparing procedure
//   const std::string body{R"(
//     <Plugin>libsequencer-pvxs.so</Plugin>
//     <RegisterType jsontype='{"type":"seq::test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
//     <Sequence>
//         <Copy input="new-pvxs-value" output="pvxs-client-variable"/>
//         <Wait timeout="1.0"/>
//     </Sequence>
//     <Workspace>
//         <PvAccessClientVariable name="pvxs-client-variable"
//             channel="PvAccessClientVariableTest:INTEGER"
//             datatype='{"type":"seq::test::Type/v1.0"}'/>
//         <Local name="new-pvxs-value"
//                type='{"type":"seq::test::Type/v1.0"}'
//                value='{"value":12.5}'/>
//     </Workspace>
// )"};
//   // preparing server
//   const std::string kDataType =
//       R"({"type":"seq::test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]})";
//   const char* channel = "PvAccessClientVariableTest:INTEGER";
//   ::ccs::base::PVAccessServer server;

//   // creating the variable
//   server.AddVariable(channel, sup::dto::AnyputVariable, kDataType.c_str());
//   server.Launch();
//   ccs::HelperTools::SleepFor(0.1 * kSecond);

//   // setting the value via PVAccessServer::GetVariable
//   sup::dto::float32 value = 42.1;
//   EXPECT_TRUE(SetAttributeValue(server.GetVariable(channel), "value", value));
//   EXPECT_TRUE(server.UpdateVariable(channel));  // without this call EXPECT below are failing
//   ccs::HelperTools::SleepFor(0.1 * kSecond);

//   // running the procedure
//   auto proc = sup::sequencer::ParseProcedureString(CreateProcedureString(body));
//   ASSERT_TRUE(proc.get() != nullptr);
//   ASSERT_TRUE(proc->Setup());

//   // executing procedure till success
//   NullUserInterface ui;
//   sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;
//   do
//   {
//     (void)ccs::HelperTools::SleepFor(0.1 * kSecond);  // Let system breathe
//     proc->ExecuteSingle(&ui);
//     exec = proc->GetStatus();
//   } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec)
//            && (sup::sequencer::ExecutionStatus::FAILURE != exec));
//   EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);

//   // reading from the server
//   sup::dto::float32 server_value = 0;
//   ASSERT_TRUE(GetAttributeValue(server.GetVariable(channel), "value", server_value));
//   EXPECT_EQ(server_value, 12.5);  // like in new-pvx-value LocalVariable
// }

PvAccessClientVariableTest::PvAccessClientVariableTest() = default;
PvAccessClientVariableTest::~PvAccessClientVariableTest() = default;