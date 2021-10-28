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

#include <SequenceParser.h>
#include <UserInterface.h>
#include <VariableRegistry.h>
#include <common/PVAccessClient.h>
#include <common/PVAccessServer.h>
#include <gtest/gtest.h>

#include <algorithm>

using ccs::HelperTools::GetAttributeValue;
using ccs::HelperTools::HasAttribute;
using ccs::HelperTools::SetAttributeValue;
using sup::sequencer::PVClientVariable;

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

class PVClientVariableTest : public ::testing::Test
{
public:
  class NullUserInterface : public sup::sequencer::UserInterface
  {
  public:
    void UpdateInstructionStatusImpl(const sup::sequencer::Instruction* instruction) {}
  };
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

//! Server creates a structure with the value.
//! Check that we can get and set the value from PVClientVariable.

TEST_F(PVClientVariableTest, PVAccessServerClientTest)
{
  const std::string kDataType =
      R"({"type":"testtype","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]})";
  const char* channel = "PVClientVariableTest:INTEGER";

  ::ccs::base::PVAccessServer server;

  // creating the variable
  server.AddVariable(channel, ccs::types::AnyputVariable, kDataType.c_str());
  server.Launch();
  ccs::HelperTools::SleepFor(0.1 * kSecond);

  // setting the value via PVAccessServer::GetVariable
  ccs::types::float32 value = 42.1;
  EXPECT_TRUE(SetAttributeValue(server.GetVariable(channel), "value", value));
  EXPECT_TRUE(server.UpdateVariable(channel));  // without this call EXPECT below are failing
  ccs::HelperTools::SleepFor(0.1 * kSecond);

  // creating the client
  ::ccs::base::PVAccessClient client;
  EXPECT_TRUE(client.AddVariable(channel, ccs::types::AnyputVariable));
  EXPECT_TRUE(client.IsValid(channel));
  EXPECT_TRUE(client.Launch());
  ccs::HelperTools::SleepFor(0.1 * kSecond);
  EXPECT_TRUE(client.IsConnected(channel));

  // reading the value using PVAccessClient::GetVariable
  ccs::types::float32 client_value = 0;
  ASSERT_TRUE(GetAttributeValue(client.GetVariable(channel), "value", client_value));
  EXPECT_EQ(value, client_value);

  // Creating sequencer's PVClientVariable
  PVClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", channel));
  EXPECT_NO_THROW(variable.AddAttribute("datatype", kDataType));

  // Reading the value from PVClientVariable
  ::ccs::types::AnyValue variable_any_value;
  EXPECT_TRUE(variable.GetValue(variable_any_value));
  EXPECT_TRUE(HasAttribute(&variable_any_value, "value"));
  ccs::types::float32 variable_value = 0;
  EXPECT_TRUE(GetAttributeValue(&variable_any_value, "value", variable_value));
  EXPECT_FLOAT_EQ(variable_value, 42.1);

  // setting the value
  variable_value = 142.0;
  EXPECT_TRUE(SetAttributeValue(&variable_any_value, "value", variable_value));
  EXPECT_TRUE(variable.SetValue(variable_any_value));
  ccs::HelperTools::SleepFor(0.1 * kSecond);

  // reading from the server
  ccs::types::float32 server_value = 0;
  ASSERT_TRUE(GetAttributeValue(server.GetVariable(channel), "value", server_value));
  EXPECT_EQ(server_value, 142.0);
}

//! Structure with value is created on server.
//! Procedure creates PVClientVariable and set new value to it.
//! We expect that the value will be propagated to the server side.

TEST_F(PVClientVariableTest, ExecuteProcedure)
{
  // preparing procedure
  const std::string body{R"(
    <Plugin>libsequencer-pvxs.so</Plugin>
    <RegisterType jsontype='{"type":"seq::test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <Sequence>
        <Copy input="new-pvxs-value" output="pvxs-client-variable"/>
        <Wait timeout="1.0"/>
    </Sequence>
    <Workspace>
        <PVClientVariable name="pvxs-client-variable"
            channel="PVClientVariableTest:INTEGER"
            datatype='{"type":"seq::test::Type/v1.0"}'/>            
        <Local name="new-pvxs-value"
               type='{"type":"seq::test::Type/v1.0"}'
               value='{"value":12.5}'/>
    </Workspace>
)"};
  // preparing server
  const std::string kDataType =
      R"({"type":"seq::test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]})";
  const char* channel = "PVClientVariableTest:INTEGER";
  ::ccs::base::PVAccessServer server;

  // creating the variable
  server.AddVariable(channel, ccs::types::AnyputVariable, kDataType.c_str());
  server.Launch();
  ccs::HelperTools::SleepFor(0.1 * kSecond);

  // setting the value via PVAccessServer::GetVariable
  ccs::types::float32 value = 42.1;
  EXPECT_TRUE(SetAttributeValue(server.GetVariable(channel), "value", value));
  EXPECT_TRUE(server.UpdateVariable(channel));  // without this call EXPECT below are failing
  ccs::HelperTools::SleepFor(0.1 * kSecond);

  // running the procedure
  auto proc = sup::sequencer::ParseProcedureString(CreateProcedureString(body));
  ASSERT_TRUE(proc.get() != nullptr);
  ASSERT_TRUE(proc->Setup());

  // executing procedure till success
  NullUserInterface ui;
  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;
  do
  {
    (void)ccs::HelperTools::SleepFor(0.1 * kSecond);  // Let system breathe
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec)
           && (sup::sequencer::ExecutionStatus::FAILURE != exec));
  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);

  // reading from the server
  ccs::types::float32 server_value = 0;
  ASSERT_TRUE(GetAttributeValue(server.GetVariable(channel), "value", server_value));
  EXPECT_EQ(server_value, 12.5);  // like in new-pvx-value LocalVariable
}
