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

#include "test_user_interface.h"
#include "unit_test_helper.h"

#include <pvxs/pv_access_read_instruction.h>

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/log_severity.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <gtest/gtest.h>

static const std::string PV_ACCESS_WRONG_OUTPUT_FIELD_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <RegisterType jsontype='{"type":"seq::wrong-field-test::channel-type","attributes":[{"value":{"type":"uint16"}}]}'/>
    <PvAccessRead name="read from pv"
                  channel="seq::read-test::variable2"
                  output="pvxs-value.value"
                  timeout="2.0"/>
    <Workspace>
        <PvAccessServer name="pvxs-variable"
                        channel="seq::read-test::variable2"
                        type='{"type":"seq::wrong-field-test::channel-type"}'/>
                        value='{"value":1.0}'/>
        <Local name="pvxs-value"
               type='{"type":"seq::wrong-field-test::channel-type"}'/>
    </Workspace>
</Procedure>)RAW";

static const std::string PV_ACCESS_READ_SUCCESS_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <RegisterType jsontype='{"type":"seq::pva_read_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <Sequence>
        <PvAccessRead name="read from pv"
                      channel="seq::read-test::variable"
                      output="pvxs-value"
                      timeout="2.0"/>
        <Equals lhs="pvxs-variable" rhs="pvxs-value"/>
    </Sequence>
    <Workspace>
        <PvAccessServer name="pvxs-variable"
                        channel="seq::read-test::variable"
                        type='{"type":"seq::pva_read_test::Type/v1.0"}'/>
                        value='{"value":1.0}'/>
        <Local name="pvxs-value"
               type='{"type":"seq::pva_read_test::Type/v1.0"}'
               value='{"value":0.0}'/>
    </Workspace>
</Procedure>)RAW";

using namespace sup::sequencer;

class PvAccessReadInstructionTest : public ::testing::Test
{
protected:
  PvAccessReadInstructionTest();
  virtual ~PvAccessReadInstructionTest();

  unit_test_helper::LogUserInterface ui;
};

TEST_F(PvAccessReadInstructionTest, Setup)
{
  Procedure proc;
  // read instruction requires a channel and an output attribute
  {
    PvAccessReadInstruction instruction{};
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("output", "Some_Var_Name"));
    EXPECT_NO_THROW(instruction.Setup(proc));
    EXPECT_NO_THROW(instruction.Reset());
  }
  // timeout attribute should be parsed to double
  {
    PvAccessReadInstruction instruction{};
    EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
    EXPECT_TRUE(instruction.AddAttribute("output", "Some_Var_Name"));
    EXPECT_TRUE(instruction.AddAttribute("timeout", "1s"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_NO_THROW(instruction.Reset());
  }
  // timeout attribute should be positive
  {
    PvAccessReadInstruction instruction{};
    EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
    EXPECT_TRUE(instruction.AddAttribute("output", "Some_Var_Name"));
    EXPECT_TRUE(instruction.AddAttribute("timeout", "-1"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_NO_THROW(instruction.Reset());
  }
}

TEST_F(PvAccessReadInstructionTest, MissingVariable)
{
  Procedure proc;
  Workspace ws;

  PvAccessReadInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("output", "DoesNotExist"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(&ui, &ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto last_log_entry = ui.m_log_entries.back();
  EXPECT_EQ(last_log_entry.first, log::SUP_SEQ_LOG_ERR);
  EXPECT_NE(last_log_entry.second.find("DoesNotExist"), std::string::npos);
}

TEST_F(PvAccessReadInstructionTest, Timeout)
{
  Procedure proc;
  Workspace ws;

  auto variable = GlobalVariableRegistry().Create("Local");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));

  PvAccessReadInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "ThisTimeouts"));
  EXPECT_TRUE(instruction.AddAttribute("output", "var"));
  EXPECT_TRUE(instruction.AddAttribute("timeout", "0.1"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(&ui, &ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto last_log_entry = ui.m_log_entries.back();
  EXPECT_EQ(last_log_entry.first, log::SUP_SEQ_LOG_WARNING);
  EXPECT_NE(last_log_entry.second.find("ThisTimeouts"), std::string::npos);
}

TEST_F(PvAccessReadInstructionTest, ProcedureWrongOutputField)
{
  EXPECT_EQ(ui.m_log_entries.size(), 0);
  auto procedure = ParseProcedureString(PV_ACCESS_WRONG_OUTPUT_FIELD_PROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  ASSERT_TRUE(procedure->Setup());

  ExecutionStatus exec = ExecutionStatus::FAILURE;
  do
  {
    procedure->ExecuteSingle(&ui);
    exec = procedure->GetStatus();
  } while ((ExecutionStatus::SUCCESS != exec) &&
           (ExecutionStatus::FAILURE != exec));
  EXPECT_EQ(exec, ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto last_log_entry = ui.m_log_entries.back();
  EXPECT_EQ(last_log_entry.first, log::SUP_SEQ_LOG_WARNING);
  EXPECT_NE(last_log_entry.second.find("pvxs-value.value"), std::string::npos);
}

TEST_F(PvAccessReadInstructionTest, ProcedureSuccess)
{
  auto procedure = ParseProcedureString(PV_ACCESS_READ_SUCCESS_PROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  ASSERT_TRUE(procedure->Setup());

  ExecutionStatus exec = ExecutionStatus::FAILURE;
  do
  {
    procedure->ExecuteSingle(&ui);
    exec = procedure->GetStatus();
  } while ((ExecutionStatus::SUCCESS != exec) && (ExecutionStatus::FAILURE != exec));
  EXPECT_EQ(exec, ExecutionStatus::SUCCESS);
}

PvAccessReadInstructionTest::PvAccessReadInstructionTest()
  : ui{}
{}

PvAccessReadInstructionTest::~PvAccessReadInstructionTest() = default;
