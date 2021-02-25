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

#include <common/ChannelAccessClient.h>

#include <SequenceParser.h>

#include <Instruction.h>
#include <InstructionRegistry.h>

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

// Function definition

TEST(ChannelAccessVariable, GetValue_success)
{

  auto variable = sup::sequencer::GlobalVariableRegistry().Create("ChannelAccessVariable");

  bool status = static_cast<bool>(variable);

  if (status)
    {
      status = Initialise();
    }

  if (status)
    { // Setup implicit with AddAttribute .. access as 'string'
      status = (variable->AddAttribute("channel", "SEQ-TEST:BOOL") &&
                variable->AddAttribute("datatype", "{\"type\":\"string\"}"));
    }

  ccs::types::AnyValue value; // Placeholder

  if (status)
    {
      status = variable->GetValue(value);
    }

  if (status)
    {
      status = (ccs::types::String == value.GetType());
    }

  if (status)
    {
      ccs::types::char8 buffer [1024];
      ccs::HelperTools::SerialiseToJSONStream(&value, buffer, 1024u);
      log_info("TEST(ChannelAccessVariable, GetValue_success) - Value is ..");
      log_info("'%s'", buffer);
    }

  (void)Terminate();

  ASSERT_EQ(true, status);

}

TEST(ChannelAccessVariable, GetValue_error)
{

  auto variable = sup::sequencer::GlobalVariableRegistry().Create("ChannelAccessVariable");

  bool status = static_cast<bool>(variable);

  if (status)
    { // Missing mandatory attribute .. Setup implicit with AddAttribute
      status = variable->AddAttribute("irrelevant", "undefined");
    }

  ccs::types::AnyValue value; // Placeholder

  if (status)
    {
      status = ((false == variable->GetValue(value)) &&
                (false == static_cast<bool>(value.GetType())));
    }

  ASSERT_EQ(true, status);

}

TEST(ChannelAccessVariable, SetValue_success)
{

  auto variable = sup::sequencer::GlobalVariableRegistry().Create("ChannelAccessVariable");

  bool status = static_cast<bool>(variable);

  if (status)
    {
      status = Initialise();
    }

  if (status)
    { // Setup implicit with AddAttribute .. access as 'float32'
      status = (variable->AddAttribute("channel", "SEQ-TEST:FLOAT") &&
                variable->AddAttribute("datatype", "{\"type\":\"float32\"}"));
    }

  ccs::types::AnyValue value (static_cast<ccs::types::float32>(0.1));

  if (status)
    {
      status = variable->SetValue(value);
    }

  if (status)
    {
      (void)ccs::HelperTools::SleepFor(100000000ul);
    }

  if (status)
    {
      status = variable->GetValue(value);
    }

  if (status)
    {
      ccs::types::char8 buffer [1024];
      ccs::HelperTools::SerialiseToJSONStream(&value, buffer, 1024u);
      log_info("TEST(ChannelAccessVariable, SetValue_success) - Value is ..");
      log_info("'%s'", buffer);
    }

  if (status)
    {
      value = static_cast<ccs::types::float32>(7.5); // Should be truncated by the IOC record
      status = variable->SetValue(value);
    }

  if (status)
    {
      (void)ccs::HelperTools::SleepFor(100000000ul);
    }

  if (status)
    {
      status = variable->GetValue(value);
    }

  if (status)
    {
      ccs::types::char8 buffer [1024];
      ccs::HelperTools::SerialiseToJSONStream(&value, buffer, 1024u);
      log_info("TEST(ChannelAccessVariable, SetValue_success) - Value is ..");
      log_info("'%s'", buffer);
    }

  (void)Terminate();

  ASSERT_EQ(true, status);

}

TEST(ChannelAccessVariable, ProcedureFile)
{

  auto proc = sup::sequencer::ParseProcedureFile("../resources/variable_ca.xml");

  bool status = static_cast<bool>(proc);

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

  ASSERT_EQ(true, status);

}

#undef LOG_ALTERN_SRC
