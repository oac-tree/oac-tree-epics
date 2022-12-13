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

#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>

#include <gtest/gtest.h>

TEST(SystemCallInstruction, Execute_missing)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("SystemCall");
  ASSERT_TRUE(static_cast<bool>(instruction));

  sup::sequencer::unit_test_helper::NullUserInterface ui;
  sup::sequencer::Procedure proc;
  EXPECT_FALSE(instruction->Setup(proc));
}

TEST(SystemCallInstruction, Execute_success)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("SystemCall");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("command", "/usr/bin/ls /tmp &> /dev/null"));

  sup::sequencer::unit_test_helper::NullUserInterface ui;
  sup::sequencer::Procedure proc;
  EXPECT_TRUE(instruction->Setup(proc));
  instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(instruction->GetStatus(), sup::sequencer::ExecutionStatus::SUCCESS);
}

TEST(SystemCallInstruction, Execute_error)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("SystemCall");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("command", "/usr/bin/undefined &> /dev/null"));

  sup::sequencer::unit_test_helper::NullUserInterface ui;
  sup::sequencer::Procedure proc;
  EXPECT_TRUE(instruction->Setup(proc));
  instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(instruction->GetStatus(), sup::sequencer::ExecutionStatus::FAILURE);
}
