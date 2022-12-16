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

#include "test_user_interface.h"

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
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
  EXPECT_NO_THROW(instruction->ExecuteSingle(&ui, &ws));
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
  EXPECT_NO_THROW(instruction->ExecuteSingle(&ui, &ws));
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::FAILURE);
}
