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
#include <common/log-api.h>

#include <common/AnyTypeDatabase.h>

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

// Global variables

static ccs::log::Func_t _log_handler = ccs::log::SetStdout();

// Function definition

TEST(RegisterTypeInstruction, Execute_missing)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("RegisterType");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::FAILURE == instruction->GetStatus());
    }

  ASSERT_EQ(true, status);

}

TEST(RegisterTypeInstruction, Execute_success)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("RegisterType");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      status = instruction->AddAttribute("datatype", "{\"type\":\"seq::test::Type/v1.0\",\"attributes\":[{\"timestamp\":{\"type\":\"uint64\"}},{\"value\":{\"type\":\"float64\"}}]}");
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::SUCCESS == instruction->GetStatus());
    }

  if (status)
    {
      status = ::ccs::base::GlobalTypeDatabase::IsValid("seq::test::Type/v1.0");
    }

  ASSERT_EQ(true, status);

}

TEST(RegisterTypeInstruction, Execute_error)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("RegisterType");

  bool status = static_cast<bool>(instruction);

  if (status)
    { // Non-terminated JSON
      status = instruction->AddAttribute("datatype", "{\"type\":\"seq::test::Type/v1.0\",\"attributes\":[{\"timestamp\":{\"type\":\"uint64\"}},{\"value\":{\"type\":\"float64\"}}]");
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
