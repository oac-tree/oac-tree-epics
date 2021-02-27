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

// Global header files

#include <gtest/gtest.h> // Google test framework

#include <common/BasicTypes.h>
#include <common/TimeTools.h>

#include <common/AnyValueHelper.h>

#include <SequenceParser.h>

#include <Instruction.h>
#include <InstructionRegistry.h>

// Local header files

#include "NullUserInterface.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "unit-test"

// Type declaration

// Function declaration

// Global variables

static ccs::log::Func_t _log_handler = ccs::log::SetStdout();

// Function definition

static inline bool Initialise (void)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("SystemCall");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      status = instruction->AddAttribute("command", "/usr/bin/screen -d -m /usr/bin/softIoc -d ../resources/ChannelAccessClient.db &> /dev/null");
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::SUCCESS == instruction->GetStatus());
    }

  return status;

}

static inline bool Terminate (void)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("SystemCall");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      status = instruction->AddAttribute("command", "/usr/bin/kill -9 `/usr/sbin/pidof softIoc` &> /dev/null");
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::SUCCESS == instruction->GetStatus());
    }

  return status;

}

TEST(ChannelAccessInstruction, Execute_missing)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::FAILURE == instruction->GetStatus());
    }

  ASSERT_EQ(true, status);

}

TEST(ChannelAccessInstruction, Execute_novar)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      status = instruction->AddAttribute("channel", "undefined");
    }

  if (status)
    {
      status = (instruction->AddAttribute("datatype", "{\"type\": \"string\"}") && instruction->AddAttribute("instance", "\"undefined\""));
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::FAILURE == instruction->GetStatus());
    }

  ASSERT_EQ(true, status);

}

TEST(ChannelAccessInstruction, Execute_success)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      status = Initialise();
    }

  if (status)
    {
      status = instruction->AddAttribute("channel", "SEQ-TEST:BOOL");
    }

  if (status)
    {
      status = (instruction->AddAttribute("datatype", "{\"type\": \"uint32\"}") && instruction->AddAttribute("instance", "1"));
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::SUCCESS == instruction->GetStatus());
    }

  (void)Terminate();

  // ToDo - Test variable

  ASSERT_EQ(true, status);

}

TEST(ChannelAccessInstruction, ProcedureFile)
{

  auto proc = sup::sequencer::ParseProcedureFile("../resources/sequence_ca.xml");

  bool status = static_cast<bool>(proc);

  if (status)
    {
      status = Initialise();
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

      do
        {
          (void)ccs::HelperTools::SleepFor(100000000ul); // Let system breathe
          proc->ExecuteSingle(&ui);
          exec = proc->GetStatus();
        }
      while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
             (sup::sequencer::ExecutionStatus::FAILURE != exec));

      status = (sup::sequencer::ExecutionStatus::SUCCESS == exec);
    }

  (void)Terminate();

  ASSERT_EQ(true, status);

}

#undef LOG_ALTERN_SRC
