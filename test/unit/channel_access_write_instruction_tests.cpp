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
#include <sup/epics-test/unit_test_helper.h>

using namespace sup::oac_tree;

static const std::string BOOLEANTYPE = R"RAW({"type":"bool"})RAW";
static const std::string BOOLEANVALUE = "true";

static const std::string BOOLEANSTRUCTTYPE = R"RAW({"type":"bool_struct","attributes":[{"value":{"type":"bool"}}]})RAW";

static const std::string UINT32ARRAYTYPE = R"RAW({"type": "uint32[8]","multiplicity":8,"element":{"type": "uint32"}})RAW";

class ChannelAccessWriteInstructionTest : public ::testing::Test
{
protected:
  ChannelAccessWriteInstructionTest();
  virtual ~ChannelAccessWriteInstructionTest();
};

TEST_F(ChannelAccessWriteInstructionTest, SetupWithVariable)
{
  Procedure proc;

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_THROW(write_instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "some_channel"));
  EXPECT_THROW(write_instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(write_instruction->AddAttribute("varName", "some_variable"));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_TRUE(write_instruction->AddAttribute("timeout", "cant_parse_this"));
  EXPECT_THROW(write_instruction->Setup(proc), InstructionSetupException);
  // Empty channel name
  EXPECT_TRUE(write_instruction->SetAttribute("channel", ""));
  EXPECT_THROW(write_instruction->Setup(proc), InstructionSetupException);
}

TEST_F(ChannelAccessWriteInstructionTest, SetupWithTypeValue)
{
  Procedure proc;

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_THROW(write_instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "some_channel"));
  EXPECT_THROW(write_instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(write_instruction->AddAttribute("type", BOOLEANTYPE));
  EXPECT_THROW(write_instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(write_instruction->AddAttribute("value", BOOLEANVALUE));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_TRUE(write_instruction->AddAttribute("timeout", "cant_parse_this"));
  EXPECT_THROW(write_instruction->Setup(proc), InstructionSetupException);
  // Empty channel name
  EXPECT_TRUE(write_instruction->SetAttribute("channel", ""));
  EXPECT_THROW(write_instruction->Setup(proc), InstructionSetupException);
}

TEST_F(ChannelAccessWriteInstructionTest, VariableNotInWorkspace)
{
  unit_test_helper::NullUserInterface ui;
  Workspace ws;
  Procedure proc;

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "undefined"));
  EXPECT_TRUE(write_instruction->AddAttribute("varName", "some_variable"));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_NO_THROW(write_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(ChannelAccessWriteInstructionTest, EmptyVariable)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("Local");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "undefined"));
  EXPECT_TRUE(write_instruction->AddAttribute("varName", "var"));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_NO_THROW(write_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(ChannelAccessWriteInstructionTest, Timeout)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("Local");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("type", BOOLEANTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "DOESNOTEXIST"));
  EXPECT_TRUE(write_instruction->AddAttribute("varName", "var"));
  EXPECT_TRUE(write_instruction->AddAttribute("timeout", "0.1"));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_NO_THROW(write_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(ChannelAccessWriteInstructionTest, TypeParseError)
{
  unit_test_helper::NullUserInterface ui;
  Workspace ws;
  Procedure proc;

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "some_channel"));
  EXPECT_TRUE(write_instruction->AddAttribute("type", "cannot parse this"));
  EXPECT_TRUE(write_instruction->AddAttribute("value", BOOLEANVALUE));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_NO_THROW(write_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(ChannelAccessWriteInstructionTest, ValueParseError)
{
  unit_test_helper::NullUserInterface ui;
  Workspace ws;
  Procedure proc;

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "some_channel"));
  EXPECT_TRUE(write_instruction->AddAttribute("type", BOOLEANTYPE));
  EXPECT_TRUE(write_instruction->AddAttribute("value", "Definitely"));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_NO_THROW(write_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(ChannelAccessWriteInstructionTest, WriteSuccess)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  // Variable to monitor result of write
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(variable->AddAttribute("type", BOOLEANTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(write_instruction->AddAttribute("type", BOOLEANTYPE));
  EXPECT_TRUE(write_instruction->AddAttribute("value", "true"));
  EXPECT_TRUE(write_instruction->AddAttribute("timeout", "5.0"));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_NO_THROW(write_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::SUCCESS);

  // TODO: the next code block is only a replacement for the commented out chech just after.
  // This needs to be changed when a solution is found for the EPICS callback issue (see sup-epics).
  {
    auto check_var = GlobalVariableRegistry().Create("ChannelAccessClient");
    ASSERT_TRUE(static_cast<bool>(check_var));
    EXPECT_TRUE(check_var->AddAttribute("channel", "SEQ-TEST:BOOL"));
    EXPECT_TRUE(check_var->AddAttribute("type", BOOLEANTYPE));
    EXPECT_NO_THROW(check_var->Setup(ws));
    EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&check_var]{
      sup::dto::AnyValue tmp;
      return check_var->GetValue(tmp) && tmp.As<bool>();
    }));
  }
  // // Check channel with variable
  // EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
  //   sup::dto::AnyValue tmp;
  //   return ws.GetValue("var", tmp) && tmp.As<bool>();
  // }));

  // Reset instruction and change value to false
  EXPECT_NO_THROW(write_instruction->Reset(ui));
  EXPECT_TRUE(write_instruction->SetAttribute("value", "false"));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_NO_THROW(write_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::SUCCESS);

  // TODO: the next code block is only a replacement for the commented out chech just after.
  // This needs to be changed when a solution is found for the EPICS callback issue (see sup-epics).
  {
    auto check_var = GlobalVariableRegistry().Create("ChannelAccessClient");
    ASSERT_TRUE(static_cast<bool>(check_var));
    EXPECT_TRUE(check_var->AddAttribute("channel", "SEQ-TEST:BOOL"));
    EXPECT_TRUE(check_var->AddAttribute("type", BOOLEANTYPE));
    EXPECT_NO_THROW(check_var->Setup(ws));
    EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&check_var]{
      sup::dto::AnyValue tmp;
      return check_var->GetValue(tmp) && !tmp.As<bool>();
    }));
  }
  // // Check channel with variable
  // EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
  //   sup::dto::AnyValue tmp;
  //   return ws.GetValue("var", tmp) && !tmp.As<bool>();
  // }));
}

TEST_F(ChannelAccessWriteInstructionTest, WriteSuccessStruct)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  Workspace ws;
  // Variable to monitor result of write
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(variable->AddAttribute("type", BOOLEANTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(write_instruction->AddAttribute("type", BOOLEANSTRUCTTYPE));
  EXPECT_TRUE(write_instruction->AddAttribute("value", R"RAW({"value":true})RAW"));
  EXPECT_NO_THROW(write_instruction->Setup(proc));
  EXPECT_NO_THROW(write_instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::SUCCESS);

  // TODO: the next code block is only a replacement for the commented out chech just after.
  // This needs to be changed when a solution is found for the EPICS callback issue (see sup-epics).
  {
    auto check_var = GlobalVariableRegistry().Create("ChannelAccessClient");
    ASSERT_TRUE(static_cast<bool>(check_var));
    EXPECT_TRUE(check_var->AddAttribute("channel", "SEQ-TEST:BOOL"));
    EXPECT_TRUE(check_var->AddAttribute("type", BOOLEANTYPE));
    EXPECT_NO_THROW(check_var->Setup(ws));
    EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&check_var]{
      sup::dto::AnyValue tmp;
      return check_var->GetValue(tmp) && tmp.As<bool>();
    }));
  }
  // // Check channel with variable
  // EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
  //   sup::dto::AnyValue tmp;
  //   return ws.GetValue("var", tmp) && tmp.As<bool>();
  // }));
}

TEST_F(ChannelAccessWriteInstructionTest, WriteArray)
{
  unit_test_helper::LogUserInterface ui;
  Procedure proc;
  Workspace ws;

  auto instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(instruction));
  // Set array
  EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:UIARRAY"));
  EXPECT_TRUE(instruction->AddAttribute("type", UINT32ARRAYTYPE));
  EXPECT_TRUE(instruction->AddAttribute("value", "[1, 2, 3, 4, 5, 6, 7, 8]"));
  EXPECT_NO_THROW(instruction->Setup(proc));
  EXPECT_NO_THROW(instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::SUCCESS);

  // Test variable
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:UIARRAY"));
  EXPECT_TRUE(variable->AddAttribute("type", UINT32ARRAYTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));

  // TODO: the next code block is only a replacement for the commented out chech just after.
  // This needs to be changed when a solution is found for the EPICS callback issue (see sup-epics).
  {
    auto check_var = GlobalVariableRegistry().Create("ChannelAccessClient");
    ASSERT_TRUE(static_cast<bool>(check_var));
    EXPECT_TRUE(check_var->AddAttribute("channel", "SEQ-TEST:UIARRAY"));
    EXPECT_TRUE(check_var->AddAttribute("type", UINT32ARRAYTYPE));
    EXPECT_NO_THROW(check_var->Setup(ws));
    EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&check_var]{
      sup::dto::AnyValue tmp;
      return check_var->GetValue(tmp) && sup::dto::IsArrayValue(tmp) && tmp[3] == 4;
    }));
  }
  // // Read from variable
  // sup::dto::AnyValue readback_val;
  // EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws, &readback_val]{
  //   return ws.GetValue("var", readback_val) && sup::dto::IsArrayValue(readback_val) &&
  //          readback_val[3] == 4;
  // }));

  // Set array back to zero
  EXPECT_NO_THROW(instruction->Reset(ui));
  EXPECT_TRUE(instruction->SetAttribute("value", "[0, 0, 0, 0, 0, 0, 0, 0]"));
  EXPECT_NO_THROW(instruction->Setup(proc));
  EXPECT_NO_THROW(instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::SUCCESS);

  // TODO: the next code block is only a replacement for the commented out check just after.
  // This needs to be changed when a solution is found for the EPICS callback issue (see sup-epics).
  {
    auto check_var = GlobalVariableRegistry().Create("ChannelAccessClient");
    ASSERT_TRUE(static_cast<bool>(check_var));
    EXPECT_TRUE(check_var->AddAttribute("channel", "SEQ-TEST:UIARRAY"));
    EXPECT_TRUE(check_var->AddAttribute("type", UINT32ARRAYTYPE));
    EXPECT_NO_THROW(check_var->Setup(ws));
    EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&check_var]{
      sup::dto::AnyValue tmp;
      return check_var->GetValue(tmp) && sup::dto::IsArrayValue(tmp) && tmp[3] == 0;
    }));
  }
  // // Read from variable
  // EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws, &readback_val]{
  //   return ws.GetValue("var", readback_val) && sup::dto::IsArrayValue(readback_val) &&
  //          readback_val[3] == 0;
  // }));

  if (!ui.m_log_entries.empty())
  {
    std::cout << ui.GetFullLog();
  }
}

TEST_F(ChannelAccessWriteInstructionTest, VariableAttributes)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <ChannelAccessWrite channel="@chan" varName="myvar" timeout="@mytimeout"/>
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

TEST_F(ChannelAccessWriteInstructionTest, VariableAttributesWrongType)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <ChannelAccessWrite channel="@chan" varName="myvar" timeout="@mytimeout"/>
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

TEST_F(ChannelAccessWriteInstructionTest, VariableAttributesNotPresent)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <ChannelAccessWrite channel="@chan" varName="myvar" timeout="@mytimeout"/>
  <Workspace>
    <Local name="chan" type='{"type":"string"}' value='"SEQ-TEST:BOOL"'/>
    <Local name="myvar" type='{"type":"bool"}'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}



ChannelAccessWriteInstructionTest::ChannelAccessWriteInstructionTest() = default;

ChannelAccessWriteInstructionTest::~ChannelAccessWriteInstructionTest() = default;
