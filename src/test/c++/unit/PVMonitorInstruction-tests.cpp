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

#include <common/PVAccessServer.h>
#include <common/PVAccessTypes.h>

#include <SequenceParser.h>

#include <Instruction.h>
#include <InstructionRegistry.h>

// Local header files

#include "SystemCall.h"

#include "NullUserInterface.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "unit-test"

// Type declaration

// Function declaration

// Global variables

static ccs::log::Func_t _log_handler = ccs::log::SetStdout();
static ccs::base::PVAccessServer* _pva_srv = static_cast<ccs::base::PVAccessServer*>(NULL);

// Function definition

static inline bool Initialise (void)
{

  _pva_srv = ccs::base::PVAccessInterface::GetInstance<ccs::base::PVAccessServer>();

  bool status = (static_cast<ccs::base::PVAccessServer*>(NULL) != _pva_srv);

  if (status)
    { // Create variable
      status = _pva_srv->AddVariable("sequencer::test::pva-variable", ccs::types::AnyputVariable, ccs::base::PVAccessTypes::Collection_int);
    }
  
  if (status)
    {
      status = _pva_srv->SetPeriod(10000000ul);
    }

  if (status)
    {
      status = _pva_srv->Launch();
    }

  if (status)
    { // Let the infrastructure settle
      (void)ccs::HelperTools::SleepFor(1000000000ul);
    }

  if (status)
    {
      status = _pva_srv->IsValid("sequencer::test::pva-variable");
    }

  return status;

}

static inline bool Terminate (void)
{

  bool status = (static_cast<ccs::base::PVAccessServer*>(NULL) != _pva_srv);

  if (status)
    {
      ccs::base::PVAccessInterface::Terminate<ccs::base::PVAccessServer>();
      _pva_srv = static_cast<ccs::base::PVAccessServer*>(NULL);
    }

  if (status)
    { // Let the infrastructure settle
      (void)ccs::HelperTools::SleepFor(1000000000ul);
    }

  return status;

}

// Function definition

TEST(PVMonitorInstruction, Setup_missing)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("PVMonitorInstruction");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::FAILURE == instruction->GetStatus());
    }

  ASSERT_EQ(true, status);

}

TEST(PVMonitorInstruction, Setup_novar)
{

  sup::sequencer::gtest::NullUserInterface ui;
  auto proc = sup::sequencer::ParseProcedureString(
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Procedure xmlns=\"http://codac.iter.org/sup/sequencer\" version=\"1.0\"\n"
    "           name=\"Trivial procedure for testing purposes\"\n"
    "           xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    "           xs:schemaLocation=\"http://codac.iter.org/sup/sequencer sequencer.xsd\">\n"
    "    <Sequence>\n"
    "        <PVMonitorInstruction name=\"pvxs-client\"\n"
    "            channel=\"sequencer::test::pva-variable\"\n"
    "            variable=\"struct\"/>\n"
    "    </Sequence>\n"
    "    <Workspace>\n"
    "        <Local name=\"struct\"/>\n"
    "    </Workspace>\n"
    "</Procedure>");

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

      status = (sup::sequencer::ExecutionStatus::FAILURE == exec);
    }

  ASSERT_EQ(true, status);

}

TEST(PVMonitorInstruction, Execute_success)
{

  sup::sequencer::gtest::NullUserInterface ui;
  auto proc = sup::sequencer::ParseProcedureString(
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Procedure xmlns=\"http://codac.iter.org/sup/sequencer\" version=\"1.0\"\n"
    "           name=\"Trivial procedure for testing purposes\"\n"
    "           xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    "           xs:schemaLocation=\"http://codac.iter.org/sup/sequencer sequencer.xsd\">\n"
    "    <Sequence>\n"
    "        <PVMonitorInstruction name=\"pvxs-client\"\n"  
    "            channel=\"sequencer::test::pva-variable\"\n"
    "            timeout=\"1000000000\"\n"
    "            variable=\"struct\"/>\n"
    "        <LogTrace input=\"struct\"/>\n"
    "    </Sequence>\n"
    "    <Workspace>\n"
    "        <FileVariable name=\"struct\" file=\"/tmp/file-variable-struct.dat\"/>\n"
    "    </Workspace>\n"
    "</Procedure>");

  bool status = static_cast<bool>(proc);

  if (status)
    { // Setup procedure
      status = proc->Setup();
    }

  if (status)
    {
      status = Initialise();
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

  (void)Terminate();

  // Test variable

  if (status)
    {
      ccs::types::AnyValue value; 
      status = ::ccs::HelperTools::ReadFromFile(&value, "/tmp/file-variable-struct.dat");

      if (status)
	{
	  status = (static_cast<bool>(value.GetType()) &&
                    ::ccs::HelperTools::HasAttribute(&value, "scalars.uint32"));
	}
    }

  // Remove temp. files
  (void)::ccs::HelperTools::ExecuteSystemCall("/usr/bin/rm -rf /tmp/file-variable-struct.dat");

  ASSERT_EQ(true, status);

}

#undef LOG_ALTERN_SRC
