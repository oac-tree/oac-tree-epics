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

#include <common/AnyTypeToCA.h>
#include <common/ChannelAccessHelper.h>

#include <SequenceParser.h>

#include <Instruction.h>
#include <InstructionRegistry.h>

// Local header files

#include "ChannelAccessClientContext.h"

#include "SystemCall.h"

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

  bool status = ::ccs::HelperTools::ExecuteSystemCall("/usr/bin/screen -d -m /usr/bin/softIoc -d ../resources/ChannelAccessClient.db &> /dev/null");

  return status;

}

static inline bool Terminate (void)
{

  bool status = ::ccs::HelperTools::ExecuteSystemCall("/usr/bin/kill -9 `/usr/sbin/pidof softIoc` &> /dev/null");

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

TEST(ChannelAccessInstruction, Fetch_boolean) // Must be associated to a variable in the workspace
{

  auto proc = sup::sequencer::ParseProcedureString(
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Procedure xmlns=\"http://codac.iter.org/sup/sequencer\" version=\"1.0\"\n"
    "           name=\"Trivial procedure for testing purposes\"\n"
    "           xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    "           xs:schemaLocation=\"http://codac.iter.org/sup/sequencer sequencer.xsd\">\n"
    "    <Sequence>\n"
    "        <ChannelAccessFetchInstruction name=\"get-client\"\n"
    "            channel=\"SEQ-TEST:BOOL\"\n"
    "            variable=\"boolean\"/>\n"
    "        <ChannelAccessFetchInstruction name=\"get-client\"\n"
    "            channel=\"SEQ-TEST:BOOL\"\n"
    "            variable=\"uint32\"/>\n"
    "        <ChannelAccessFetchInstruction name=\"get-client\"\n"
    "            channel=\"SEQ-TEST:BOOL\"\n"
    "            variable=\"string\"/>\n"
    "    </Sequence>\n"
    "    <Workspace>\n"
    "        <FileVariable name=\"boolean\" file=\"/tmp/file-variable-boolean.dat\"/>\n"
    "        <FileVariable name=\"uint32\" file=\"/tmp/file-variable-uint32.dat\"/>\n"
    "        <FileVariable name=\"string\" file=\"/tmp/file-variable-string.dat\"/>\n"
    "    </Workspace>\n"
    "</Procedure>");

  bool status = static_cast<bool>(proc);

  if (status)
    {
      status = Initialise();
    }

  if (status)
    { // Create file to initialise the waorkspace variables with right datatype
      ccs::types::AnyValue boolean_var (false); ccs::HelperTools::DumpToFile(&boolean_var, "/tmp/file-variable-boolean.dat");
      ccs::types::AnyValue uint32_var (0u); ccs::HelperTools::DumpToFile(&uint32_var, "/tmp/file-variable-uint32.dat");
      ccs::types::AnyValue string_var ("undefined"); ccs::HelperTools::DumpToFile(&string_var, "/tmp/file-variable-string.dat");
    }

  if (status)
    {
      (void)::ccs::HelperTools::SleepFor(100000000ul);
      status = ::ccs::HelperTools::ExecuteSystemCall("/usr/bin/caput SEQ-TEST:BOOL TRUE");
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

  // Test variable

  if (status)
    {
      ccs::types::AnyValue value; 
      status = ccs::HelperTools::ReadFromFile(&value, "/tmp/file-variable-boolean.dat");

      if (status)
	{
	  status = (true == static_cast<bool>(value));
	}
    }

  if (status)
    {
      ccs::types::AnyValue value; 
      status = ccs::HelperTools::ReadFromFile(&value, "/tmp/file-variable-uint32.dat");

      if (status)
	{
	  status = (1u == static_cast<ccs::types::uint32>(value));
	}
    }

  // Remove temp. files
  (void)::ccs::HelperTools::ExecuteSystemCall("/usr/bin/rm -rf /tmp/file-variable-boolean.dat");
  (void)::ccs::HelperTools::ExecuteSystemCall("/usr/bin/rm -rf /tmp/file-variable-uint32.dat");
  (void)::ccs::HelperTools::ExecuteSystemCall("/usr/bin/rm -rf /tmp/file-variable-string.dat");

  (void)Terminate();

  ASSERT_EQ(true, status);

}

TEST(ChannelAccessInstruction, Write_boolean)
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

  // Test variable

  // At this point, the instruction has diconnected form the channel and thread detached from the context

  if (status)
    {
      status = ccs::HelperTools::ChannelAccessClientContext::Attach();
    }

  chid channel;

  if (status)
    {
      (void)ccs::HelperTools::ChannelAccess::ConnectVariable("SEQ-TEST:BOOL", channel);
      (void)ccs::HelperTools::SleepFor(100000000ul);
      status = ccs::HelperTools::ChannelAccess::IsConnected(channel);
    }

  ccs::types::AnyValue value (false);

  if (status)
    {
      status = ccs::HelperTools::ChannelAccess::ReadVariable(channel, ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), value.GetInstance());
    }

  if (status)
    {
      log_info("TEST(ChannelAccessInstruction, Execute_boolean) - Test variable ..");
      status = (true == static_cast<bool>(value));
    }

  (void)ccs::HelperTools::ChannelAccess::DetachVariable(channel);
  (void)ccs::HelperTools::ChannelAccessClientContext::Detach();

  (void)Terminate();

  ASSERT_EQ(true, status);

}

TEST(ChannelAccessInstruction, Write_float32)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      status = Initialise();
    }

  if (status)
    {
      status = instruction->AddAttribute("channel", "SEQ-TEST:FLOAT");
    }

  if (status)
    {
      status = (instruction->AddAttribute("datatype", "{\"type\": \"string\"}") && instruction->AddAttribute("instance", "\"0.5\""));
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::SUCCESS == instruction->GetStatus());
    }

  // Test variable

  // At this point, the instruction has diconnected form the channel and thread detached from the context

  if (status)
    {
      status = ccs::HelperTools::ChannelAccessClientContext::Attach();
    }

  chid channel;

  if (status)
    {
      (void)ccs::HelperTools::ChannelAccess::ConnectVariable("SEQ-TEST:FLOAT", channel);
      (void)ccs::HelperTools::SleepFor(100000000ul);
      status = ccs::HelperTools::ChannelAccess::IsConnected(channel);
    }

  ccs::types::AnyValue value (static_cast<ccs::types::float32>(0.0));

  if (status)
    {
      status = ccs::HelperTools::ChannelAccess::ReadVariable(channel, ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), value.GetInstance());
    }

  if (status)
    {
      log_info("TEST(ChannelAccessInstruction, Execute_float32) - Test variable ..");
      status = (static_cast<ccs::types::float32>(0.5) == static_cast<ccs::types::float32>(value));
    }

  (void)ccs::HelperTools::ChannelAccess::DetachVariable(channel);
  (void)ccs::HelperTools::ChannelAccessClientContext::Detach();

  (void)Terminate();

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
