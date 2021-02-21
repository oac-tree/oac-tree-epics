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

// Global header files

#include <gtest/gtest.h> // Google test framework

#include <common/BasicTypes.h>

//#include <SequenceParser.h>
#include <Instruction.h>
#include <InstructionRegistry.h>

// Local header files

#include "NullUserInterface.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "unit-test"

// Type declaration

// Function declaration

// Function definition

TEST(SystemCallInstruction, Execute_missing)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("SystemCall");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::FAILURE == instruction->GetStatus());
    }

  ASSERT_EQ(true, status);
}

TEST(SystemCallInstruction, Execute_success)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("SystemCall");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      status = instruction->AddAttribute("command","ls /tmp &> /dev/null");
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::SUCCESS == instruction->GetStatus());
    }

  ASSERT_EQ(true, status);
}

TEST(SystemCallInstruction, Execute_error)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("SystemCall");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      status = instruction->AddAttribute("command","/usr/bin/undefined &> /dev/null");
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::FAILURE == instruction->GetStatus());
    }

  ASSERT_EQ(true, status);
}

#undef LOG_ALTERN_SRC
