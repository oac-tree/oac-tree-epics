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

#include <pvxs/pv_access_server_variable.h>

#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/variable.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/json_type_parser.h>

#include <gtest/gtest.h>

static const std::string PVACCESSWRITEPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <Repeat maxCount="3">
        <Sequence>
            <Wait name="wait"
                timeout="0.2"/>
            <Copy input="curr-time" output="pvxs-variable.timestamp"/>
            <Copy input="curr-time" output="pvxs-other.timestamp"/>
            <LogTrace name="variable"
                input="pvxs-variable"/>
            <LogTrace name="other"
                input="pvxs-other"/>
        </Sequence>
    </Repeat>
    <Workspace>
        <SystemClock name="curr-time"/>
        <PVServerVariable name="pvxs-variable"
            channel="seq::test::variable"
            datatype='{"type":"seq::test::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'
            instance='{"timestamp":0,"value":0.1}'/>
        <PVServerVariable name="pvxs-other"
            channel="seq::test::another"
            datatype='{"type":"seq::test::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'
            instance='{"timestamp":0,"value":1.0}'/>
    </Workspace>
</Procedure>)RAW";

using namespace sup::sequencer;

class PvAccessServerVariableTest : public ::testing::Test
{
protected:
  PvAccessServerVariableTest();
  ~PvAccessServerVariableTest();
};

TEST_F(PvAccessServerVariableTest, VariableRegistration)
{
  auto registry = sup::sequencer::GlobalVariableRegistry();
  auto names = registry.RegisteredVariableNames();
  ASSERT_TRUE(std::find(names.begin(), names.end(), PvAccessServerVariable::Type) != names.end());
  EXPECT_TRUE(dynamic_cast<PvAccessServerVariable*>(registry.Create(PvAccessServerVariable::Type).get()));
}

TEST_F(PvAccessServerVariableTest, InvalidSetup)
{
  PvAccessServerVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PvAccessClientVariableTest:INTEGER"));
  EXPECT_NO_THROW(variable.AddAttribute("type", "invalid-type"));
  EXPECT_NO_THROW(variable.Setup());
}

TEST_F(PvAccessServerVariableTest, ValidSetup)
{
  PvAccessServerVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PvAccessClientVariableTest:INTEGER"));
  EXPECT_NO_THROW(variable.AddAttribute("type", R"RAW({"type":"uint64"})RAW"));
  EXPECT_NO_THROW(variable.Setup());

  sup::dto::AnyValue value;
  EXPECT_TRUE(variable.GetValue(value));
  EXPECT_EQ(value.GetType(), sup::dto::UnsignedInteger64Type);
}

//! Server creates a structure with the value.
//! Check that we can get and set the value from PvAccessClientVariable.

TEST_F(PvAccessServerVariableTest, PvAccessServerClientTest)
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
  EXPECT_TRUE(ws.AddVariable("server_var", server_var.release()));

  // Creating sequencer's PvAccessClientVariable
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", channel));
  EXPECT_NO_THROW(variable->AddAttribute("type", kDataType));
  EXPECT_TRUE(ws.AddVariable("client_var", variable.release()));
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
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws, new_value]{
    sup::dto::AnyValue server_val;
    return ws.GetValue("server_var", server_val) &&
           server_val.HasField("value") &&
           server_val["value"] == new_value;
  }));
}

// TEST(PVServerVariable, ProcedureVariable)
// {
//   sup::sequencer::gtest::NullUserInterface ui;
//   auto proc = sup::sequencer::ParseProcedureString(PVACCESSWRITEPROCEDURE);
//   ASSERT_TRUE(static_cast<bool>(proc));
//   ASSERT_TRUE(proc->Setup());

//   sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

//   do
//   {
//     (void)ccs::HelperTools::SleepFor(100000000ul); // Let system breathe
//     proc->ExecuteSingle(&ui);
//     exec = proc->GetStatus();
//   } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
//            (sup::sequencer::ExecutionStatus::FAILURE != exec));

//   EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);
// }

PvAccessServerVariableTest::PvAccessServerVariableTest() = default;
PvAccessServerVariableTest::~PvAccessServerVariableTest() = default;
