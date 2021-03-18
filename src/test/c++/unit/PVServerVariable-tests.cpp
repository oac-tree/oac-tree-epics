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
#include <Variable.h>
#include <VariableRegistry.h>

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

  return true;

}

static inline bool Terminate (void)
{

  return true;

}

// Function definition

TEST(PVServerVariable, ProcedureVariable)
{

  std::string file; // Placeholder

  if (::ccs::HelperTools::Exist("../resources/variable_pvxs.xml"))
    {
      file = std::string("../resources/variable_pvxs.xml");
    }
  else
    {
      file = std::string("./target/test/resources/variable_pvxs.xml");
    }

  sup::sequencer::gtest::NullUserInterface ui;
  auto proc = sup::sequencer::ParseProcedureFile(file);

  bool status = static_cast<bool>(proc);

  if (status)
    { // Setup procedure
      status = proc->Setup();
    }

  if (status)
    {
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

  ASSERT_EQ(true, status);

}

#undef LOG_ALTERN_SRC
