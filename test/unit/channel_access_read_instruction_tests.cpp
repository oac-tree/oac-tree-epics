/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP oac-tree
 *
 * Description   : Unit test code
 *
 * Author        : B.Bauvir (IO)
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

#include "test_user_interface.h"
#include "unit_test_helper.h"

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/instruction.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/procedure.h>
#include <sup/oac-tree/sequence_parser.h>
#include <sup/oac-tree/variable.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <gtest/gtest.h>
#include <sup/epics-test/softioc_utils.h>

using namespace sup::oac_tree;

static const std::string WRONGSTRUCTTYPE =
  R"RAW({"type":"wrongstruct","attributes":[{"flag":{"type":"bool"}}]})RAW";

static const std::string WRONGCONNECTEDTYPE =
  R"RAW({"type":"wrongextrafield","attributes":[{"value":{"type":"bool"}},{"connected":{"type":"string"}}]})RAW";

static const std::string WRONGTIMESTAMPTYPE =
  R"RAW({"type":"wrongextrafield","attributes":[{"value":{"type":"bool"}},{"timestamp":{"type":"string"}}]})RAW";

static const std::string WRONGSTATUSTYPE =
  R"RAW({"type":"wrongextrafield","attributes":[{"value":{"type":"bool"}},{"status":{"type":"string"}}]})RAW";

static const std::string WRONGSEVERITYTYPE =
  R"RAW({"type":"wrongextrafield","attributes":[{"value":{"type":"bool"}},{"severity":{"type":"string"}}]})RAW";

static const std::string READBOOLPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/oac-tree" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/oac-tree oac-tree.xsd">
    <Sequence>
        <ChannelAccessRead name="get-client"
            channel="SEQ-TEST:BOOL"
            outputVar="boolean"/>
        <ChannelAccessRead name="get-client"
            channel="SEQ-TEST:BOOL"
            outputVar="int32"/>
        <ChannelAccessRead name="get-client"
            channel="SEQ-TEST:BOOL"
            outputVar="string"/>
    </Sequence>
    <Workspace>
        <Local name="boolean" type='{"type":"bool"}'/>
        <Local name="int32" type='{"type":"int32"}'/>
        <Local name="string" type='{"type":"string"}'/>
    </Workspace>
</Procedure>)RAW";

class ChannelAccessReadInstructionTest : public ::testing::Test
{
protected:
  ChannelAccessReadInstructionTest();
  virtual ~ChannelAccessReadInstructionTest();
};

TEST_F(ChannelAccessReadInstructionTest, Setup)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));
  EXPECT_THROW(read_instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "some_channel"));
  EXPECT_THROW(read_instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "some_var_name"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "cant_parse_this"));
  EXPECT_THROW(read_instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(read_instruction->SetAttribute("timeout", "30.0"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  EXPECT_NO_THROW(read_instruction->Reset(ui));
}

TEST_F(ChannelAccessReadInstructionTest, ExecuteMissingVariable)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  EXPECT_NO_THROW(ws.Setup());

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));

  // Variable to write to doesn't exist in workspace
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "some_var_name"));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "1.0"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  EXPECT_NO_THROW(read_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(read_instruction->GetStatus(), ExecutionStatus::FAILURE);
  EXPECT_NO_THROW(read_instruction->Reset(ui));
}

TEST_F(ChannelAccessReadInstructionTest, ExecuteEmptyVariable)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("Local");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));

  // Variable to write to doesn't exist in workspace
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "var"));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "1.0"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  EXPECT_NO_THROW(read_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(read_instruction->GetStatus(), ExecutionStatus::FAILURE);
  EXPECT_NO_THROW(read_instruction->Reset(ui));

  sup::dto::AnyValue value;
  EXPECT_FALSE(ws.GetValue("var", value));
}

TEST_F(ChannelAccessReadInstructionTest, ExecuteEmptyChannelType)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("Local");

  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("type", WRONGSTRUCTTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));

  // Variable to write to doesn't exist in workspace
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "var"));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "1.0"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  EXPECT_NO_THROW(read_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(read_instruction->GetStatus(), ExecutionStatus::FAILURE);
  EXPECT_NO_THROW(read_instruction->Reset(ui));

  sup::dto::AnyValue value;
  EXPECT_TRUE(ws.GetValue("var", value));
  EXPECT_TRUE(value.HasField("flag"));
}

TEST_F(ChannelAccessReadInstructionTest, Timeout)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("Local");

  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("type", R"RAW({"type":"bool"})RAW"));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));

  // Variable to write to doesn't exist in workspace
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "DOESNOTEXIST"));
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "var"));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "0.1"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  while (!IsFinishedStatus(read_instruction->GetStatus()))
  {
    EXPECT_NO_THROW(read_instruction->ExecuteSingle(ui, ws));
  }
  EXPECT_EQ(read_instruction->GetStatus(), ExecutionStatus::FAILURE);
  EXPECT_NO_THROW(read_instruction->Reset(ui));

  sup::dto::AnyValue value;
  EXPECT_TRUE(ws.GetValue("var", value));
  EXPECT_EQ(value.GetType(), sup::dto::BooleanType);
}

TEST_F(ChannelAccessReadInstructionTest, WrongConnectedType)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("Local");

  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("type", WRONGCONNECTEDTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));

  // Variable to write to doesn't exist in workspace
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "var"));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "2.0"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  while (!IsFinishedStatus(read_instruction->GetStatus()))
  {
    EXPECT_NO_THROW(read_instruction->ExecuteSingle(ui, ws));
  }
  EXPECT_EQ(read_instruction->GetStatus(), ExecutionStatus::FAILURE);
  EXPECT_NO_THROW(read_instruction->Reset(ui));

  sup::dto::AnyValue value;
  EXPECT_TRUE(ws.GetValue("var", value));
  EXPECT_TRUE(value.HasField("value"));
  EXPECT_TRUE(value.HasField("connected"));
}

TEST_F(ChannelAccessReadInstructionTest, WrongTimestampType)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("Local");

  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("type", WRONGTIMESTAMPTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));

  // Variable to write to doesn't exist in workspace
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "var"));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "2.0"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  while (!IsFinishedStatus(read_instruction->GetStatus()))
  {
    EXPECT_NO_THROW(read_instruction->ExecuteSingle(ui, ws));
  }
  EXPECT_EQ(read_instruction->GetStatus(), ExecutionStatus::FAILURE);
  EXPECT_NO_THROW(read_instruction->Reset(ui));

  sup::dto::AnyValue value;
  EXPECT_TRUE(ws.GetValue("var", value));
  EXPECT_TRUE(value.HasField("value"));
  EXPECT_TRUE(value.HasField("timestamp"));
}

TEST_F(ChannelAccessReadInstructionTest, WrongStatusType)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("Local");

  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("type", WRONGSTATUSTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));

  // Variable to write to doesn't exist in workspace
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "var"));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "2.0"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  while (!IsFinishedStatus(read_instruction->GetStatus()))
  {
    EXPECT_NO_THROW(read_instruction->ExecuteSingle(ui, ws));
  }
  EXPECT_EQ(read_instruction->GetStatus(), ExecutionStatus::FAILURE);
  EXPECT_NO_THROW(read_instruction->Reset(ui));

  sup::dto::AnyValue value;
  EXPECT_TRUE(ws.GetValue("var", value));
  EXPECT_TRUE(value.HasField("value"));
  EXPECT_TRUE(value.HasField("status"));
}

TEST_F(ChannelAccessReadInstructionTest, WrongSeverityType)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("Local");

  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("type", WRONGSEVERITYTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));

  // Variable to write to doesn't exist in workspace
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "var"));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "2.0"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  while (!IsFinishedStatus(read_instruction->GetStatus()))
  {
    EXPECT_NO_THROW(read_instruction->ExecuteSingle(ui, ws));
  }
  EXPECT_EQ(read_instruction->GetStatus(), ExecutionStatus::FAILURE);
  EXPECT_NO_THROW(read_instruction->Reset(ui));

  sup::dto::AnyValue value;
  EXPECT_TRUE(ws.GetValue("var", value));
  EXPECT_TRUE(value.HasField("value"));
  EXPECT_TRUE(value.HasField("severity"));
}

TEST_F(ChannelAccessReadInstructionTest, ReadOnlyVariable)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  sup::dto::AnyValue val = true;
  auto variable = std::make_unique<unit_test_helper::ReadOnlyVariable>(val);
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));

  // Variable to write to doesn't exist in workspace
  EXPECT_TRUE(read_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(read_instruction->AddAttribute("outputVar", "var"));
  EXPECT_TRUE(read_instruction->AddAttribute("timeout", "2.0"));
  EXPECT_NO_THROW(read_instruction->Setup(proc));
  while (!IsFinishedStatus(read_instruction->GetStatus()))
  {
    EXPECT_NO_THROW(read_instruction->ExecuteSingle(ui, ws));
  }
  EXPECT_EQ(read_instruction->GetStatus(), ExecutionStatus::FAILURE);
  EXPECT_NO_THROW(read_instruction->Reset(ui));

  sup::dto::AnyValue value;
  EXPECT_TRUE(ws.GetValue("var", value));
  EXPECT_EQ(value.GetType(), sup::dto::BooleanType);
}

TEST_F(ChannelAccessReadInstructionTest, ReadBoolean)
{
  unit_test_helper::NullUserInterface ui;
  auto proc = ParseProcedureString(READBOOLPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(proc));
  EXPECT_NO_THROW(proc->Setup());

  ASSERT_TRUE(unit_test_helper::WaitForCAChannel("SEQ-TEST:BOOL", R"RAW({"type":"bool"})RAW", 5.0));

  std::string command = sup::epics::test::GetEPICSExecutablePath("caput") + " SEQ-TEST:BOOL TRUE";
  EXPECT_TRUE(std::system(command.c_str()) == 0);

  ExecutionStatus exec = ExecutionStatus::FAILURE;
  do
  {
    proc->ExecuteSingle(ui);
    exec = proc->GetStatus();
  } while ((ExecutionStatus::SUCCESS != exec) && (ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, ExecutionStatus::SUCCESS);

  auto& ws = proc->GetWorkspace();

  // test boolean variable
  sup::dto::AnyValue bool_var;
  EXPECT_TRUE(ws.GetValue("boolean", bool_var));
  EXPECT_TRUE(bool_var == true);

  // test boolean variable
  sup::dto::AnyValue uint32_var;
  EXPECT_TRUE(ws.GetValue("int32", uint32_var));
  EXPECT_TRUE(uint32_var == 1);

  // test boolean variable
  sup::dto::AnyValue string_var;
  EXPECT_TRUE(ws.GetValue("string", string_var));
  EXPECT_TRUE(string_var == "TRUE");
}

TEST_F(ChannelAccessReadInstructionTest, VariableAttributes)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <ChannelAccessRead channel="@chan" outputVar="myvar" timeout="@mytimeout"/>
  <Workspace>
    <Local name="chan" type='{"type":"string"}' value='"SEQ-TEST:BOOL"'/>
    <Local name="mytimeout" type='{"type":"float64"}' value='3.0'/>
    <Local name="myvar" type='{"type":"bool"}'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui));
}

TEST_F(ChannelAccessReadInstructionTest, VariableAttributesWrongType)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <ChannelAccessRead channel="@chan" outputVar="myvar" timeout="@mytimeout"/>
  <Workspace>
    <Local name="chan" type='{"type":"float64"}' value='4.3'/>
    <Local name="mytimeout" type='{"type":"float64"}' value='3.0'/>
    <Local name="myvar" type='{"type":"bool"}'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}

TEST_F(ChannelAccessReadInstructionTest, VariableAttributesNotPresent)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <ChannelAccessRead channel="@chan" outputVar="myvar" timeout="@mytimeout"/>
  <Workspace>
    <Local name="chan" type='{"type":"string"}' value='"SEQ-TEST:BOOL"'/>
    <Local name="myvar" type='{"type":"bool"}'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}

ChannelAccessReadInstructionTest::ChannelAccessReadInstructionTest() = default;

ChannelAccessReadInstructionTest::~ChannelAccessReadInstructionTest() = default;
