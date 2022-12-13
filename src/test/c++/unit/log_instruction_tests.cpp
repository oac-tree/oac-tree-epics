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
 * Copyright (c) : 2010-2022 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include "null_user_interface.h"

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/sequence_parser.h>

#include <gtest/gtest.h>

#include <sstream>

using namespace sup::sequencer;

static const std::string LOGVARIABLEPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <Sequence>
        <Copy input="time" output="iso_time"/>
        <Log input="iso_time" severity="emergency"/>
        <Copy input="time" output="iso_time"/>
        <Log input="iso_time" severity="alert"/>
        <Copy input="time" output="iso_time"/>
        <Log input="iso_time" severity="critical"/>
        <Copy input="time" output="iso_time"/>
        <Log input="iso_time" severity="error"/>
        <Copy input="time" output="iso_time"/>
        <Log input="iso_time" severity="warning"/>
        <Copy input="time" output="iso_time"/>
        <Log input="iso_time" severity="notice"/>
        <Copy input="time" output="iso_time"/>
        <Log input="iso_time" severity="info"/>
        <Copy input="time" output="iso_time"/>
        <Log input="iso_time" severity="debug"/>
        <Copy input="time" output="iso_time"/>
        <Log input="iso_time" severity="trace"/>
    </Sequence>
    <Workspace>
        <SystemClock name="time" format="ISO8601"/>
        <Local name="iso_time" type='{"type":"string"}'/>
    </Workspace>
</Procedure>)RAW";

class LogInstructionTest : public ::testing::Test
{
protected:
  LogInstructionTest();
  virtual ~LogInstructionTest();
};

TEST_F(LogInstructionTest, Setup)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("Log");
  ASSERT_TRUE(static_cast<bool>(instruction));

  Procedure proc;
  std::string log_message = "Hello test!";
  EXPECT_THROW(instruction->Setup(proc), InstructionSetupException);
  EXPECT_TRUE(instruction->AddAttribute("message", log_message));
  EXPECT_NO_THROW(instruction->Setup(proc));
}

TEST_F(LogInstructionTest, Message)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("Log");
  ASSERT_TRUE(static_cast<bool>(instruction));

  Procedure proc;
  std::string log_message = "Hello test!";
  EXPECT_TRUE(instruction->AddAttribute("message", log_message));
  EXPECT_NO_THROW(instruction->Setup(proc));

  std::ostringstream oss;
  log::LogStreamRedirector redirector(oss);
  unit_test_helper::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_NE(oss.str().find(log_message), std::string::npos);
}

TEST_F(LogInstructionTest, VariableInput)
{
  unit_test_helper::NullUserInterface ui;
  auto proc = ParseProcedureString(LOGVARIABLEPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(proc));
  ASSERT_TRUE(proc->Setup());

  std::ostringstream oss;
  log::LogStreamRedirector redirector(oss);
  ExecutionStatus exec = ExecutionStatus::FAILURE;
  do
  {
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((ExecutionStatus::SUCCESS != exec) && (ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, ExecutionStatus::SUCCESS);
  EXPECT_NE(oss.str().find("ERROR"), std::string::npos);
  EXPECT_NE(oss.str().find("WARNING"), std::string::npos);
  EXPECT_NE(oss.str().find("sup::sequencer"), std::string::npos);
}

LogInstructionTest::LogInstructionTest()
{
  log::SetMaxSeverity(log::SUP_LOG_TRACE);
}

LogInstructionTest::~LogInstructionTest()
{
  log::SetMaxSeverity(log::SUP_LOG_DEBUG);
}
