/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP Sequencer
 *
 * Description   : Unit test code
 *
 * Author        : B.Bauvir (IO)
 *
 * Copyright (c) : 2010-2025 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include "test_user_interface.h"
#include "unit_test_helper.h"

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/workspace.h>

#include <gtest/gtest.h>

using namespace sup::sequencer;

TEST(SystemCallInstruction, Setup)
{
  Procedure proc;

  auto instruction = GlobalInstructionRegistry().Create("SystemCall");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_THROW(instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(instruction->AddAttribute("command", "not_relevant"));
  EXPECT_NO_THROW(instruction->Setup(proc));
}

TEST(SystemCallInstruction, Success)
{
  auto instruction = GlobalInstructionRegistry().Create("SystemCall");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("command", "/usr/bin/ls /tmp &> /dev/null"));

  unit_test_helper::NullUserInterface ui;
  Workspace ws;
  Procedure proc;
  EXPECT_NO_THROW(instruction->Setup(proc));
  EXPECT_NO_THROW(instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::SUCCESS);
}

TEST(SystemCallInstruction, Failure)
{
  auto instruction = GlobalInstructionRegistry().Create("SystemCall");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("command", "/usr/bin/undefined &> /dev/null"));

  unit_test_helper::NullUserInterface ui;
  Workspace ws;
  Procedure proc;
  EXPECT_NO_THROW(instruction->Setup(proc));
  EXPECT_NO_THROW(instruction->ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST(SystemCallInstruction, VariableCommand)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <SystemCall command="@ls"/>
  <Workspace>
    <Local name="ls" type='{"type":"string"}' value='"echo \"Hello\""'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui));
}

TEST(SystemCallInstruction, VariableCommandWrongType)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <SystemCall command="@ls"/>
  <Workspace>
    <Local name="ls" type='{"type":"float32"}' value='4.3'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}

TEST(SystemCallInstruction, VariableCommandNotPresent)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <SystemCall command="@ls"/>
  <Workspace>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}
