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

#include <SequenceParser.h>

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

TEST(ParallelSequence, Procedure_sequence)
{

  bool status = true;

  try
    {
      auto proc = sup::sequencer::ParseProcedureString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Procedure xmlns=\"http://codac.iter.org/sup/sequencer\" version=\"1.0\"\n"
        "           name=\"Trivial procedure for testing purposes\"\n"
        "           xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
        "           xs:schemaLocation=\"http://codac.iter.org/sup/sequencer sequencer.xsd\">\n"
        "    <Repeat maxCount=\"10\">\n"
        "        <Sequence>\n"
        "            <Wait name=\"wait\" timeout=\"0.1\"/>\n"
        "            <LogTrace input=\"time\"/>\n"
        "        </Sequence>\n"
        "    </Repeat>\n"
        "    <Workspace>\n"
        "        <SystemClock name=\"time\" datatype='{\"type\":\"string\"}'/>\n"
        "    </Workspace>\n"
        "</Procedure>");

      if (status)
        {
          status = static_cast<bool>(proc);
        }

      if (status)
        {
          sup::sequencer::gtest::NullUserInterface ui;
          sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

          do
            {
              (void)ccs::HelperTools::SleepFor(10000000ul); // Let system breathe
              proc->ExecuteSingle(&ui);
              exec = proc->GetStatus();
            }
          while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
                 (sup::sequencer::ExecutionStatus::FAILURE != exec));

          status = (sup::sequencer::ExecutionStatus::SUCCESS == exec);
        }
    }
  catch (const std::exception& e)
    { // Exception caught
      log_error("TEST(ParallelSequence, Procedure_sequence) - .. '%s' exception caught", e.what());
      status = false;
    }
  catch (...)
    { // Exception caught
      log_error("TEST(ParallelSequence, Procedure_sequence) - .. unknown exception caught");
      status = false;
    }

  ASSERT_EQ(true, status);

}
// Issue during the tear-down process
TEST(ParallelSequence, Procedure_parallel)
{

  bool status = true;

  try
    {
      auto proc = sup::sequencer::ParseProcedureString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<Procedure xmlns=\"http://codac.iter.org/sup/sequencer\" version=\"1.0\"\n"
        "           name=\"Trivial procedure for testing purposes\"\n"
        "           xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
        "           xs:schemaLocation=\"http://codac.iter.org/sup/sequencer sequencer.xsd\">\n"
        "    <Repeat maxCount=\"10\">\n"
        "        <ParallelSequence>\n"
        "            <Wait name=\"wait\" timeout=\"0.1\"/>\n"
        "            <LogTrace input=\"time\"/>\n"
        "        </ParallelSequence>\n"
        "    </Repeat>\n"
        "    <Workspace>\n"
        "        <SystemClock name=\"time\" datatype='{\"type\":\"string\"}'/>\n"
        "    </Workspace>\n"
        "</Procedure>");

      if (status)
        {
          status = static_cast<bool>(proc);
        }

      if (status)
        {
          sup::sequencer::gtest::NullUserInterface ui;
          sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

          do
            {
              (void)ccs::HelperTools::SleepFor(10000000ul); // Let system breathe
              proc->ExecuteSingle(&ui);
              exec = proc->GetStatus();
            }
          while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
                 (sup::sequencer::ExecutionStatus::FAILURE != exec));

          status = (sup::sequencer::ExecutionStatus::SUCCESS == exec);
        }
    }
  catch (const std::exception& e)
    { // Exception caught
      log_error("TEST(ParallelSequence, Procedure_parallel) - .. '%s' exception caught", e.what());
      status = false;
    }
  catch (...)
    { // Exception caught
      log_error("TEST(ParallelSequence, Procedure_parallel) - .. unknown exception caught");
      status = false;
    }

  ASSERT_EQ(true, status);

}

#undef LOG_ALTERN_SRC
