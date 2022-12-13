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

#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <gtest/gtest.h>

static const std::string PVACCESSSERVERPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
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

static const std::string PVACCESSMISSINGCHANNELPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <RegisterType jsontype='{"type":"seq::missing-channel-test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <Sequence>
        <PvAccessRead name="read from pv"
            channel="seq::test::missing-channel"
            output="pvxs-value"
            timeout="0.3"/>
    </Sequence>
    <Workspace>
        <Local name="pvxs-value"
               type='{"type":"seq::missing-channel-test::Type/v1.0"}'/>
    </Workspace>
</Procedure>)RAW";

using namespace sup::sequencer;

class PvAccessReadInstructionTest : public ::testing::Test
{
protected:
  PvAccessReadInstructionTest();
  virtual ~PvAccessReadInstructionTest();

  unit_test_helper::NullUserInterface ui;
};

TEST_F(PvAccessReadInstructionTest, read_success)
{
  auto procedure = sup::sequencer::ParseProcedureString(PVACCESSSERVERPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  ASSERT_TRUE(procedure->Setup());

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    procedure->ExecuteSingle(&ui);
    exec = procedure->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);
}

TEST_F(PvAccessReadInstructionTest, missing_attributes)
{
  sup::sequencer::Procedure proc{};
  proc.AddVariable("SOME_VARIABLE",
    sup::sequencer::GlobalVariableRegistry().Create("Local").release());
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("PvAccessRead");
  ASSERT_TRUE(static_cast<bool>(instruction));
  EXPECT_FALSE(instruction->Setup(proc));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SOME_CHANNEL"));
  EXPECT_FALSE(instruction->Setup(proc));
  EXPECT_TRUE(instruction->AddAttribute("output", "SOME_VARIABLE"));
  EXPECT_TRUE(instruction->Setup(proc));
}

TEST_F(PvAccessReadInstructionTest, missing_variable)
{
  sup::sequencer::Procedure proc{};
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("PvAccessRead");
  ASSERT_TRUE(static_cast<bool>(instruction));
  EXPECT_FALSE(instruction->Setup(proc));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SOME_CHANNEL"));
  EXPECT_FALSE(instruction->Setup(proc));
  EXPECT_TRUE(instruction->AddAttribute("output", "SOME_VARIABLE"));
  EXPECT_FALSE(instruction->Setup(proc));
  proc.AddVariable("SOME_VARIABLE",
    sup::sequencer::GlobalVariableRegistry().Create("Local").release());
  EXPECT_TRUE(instruction->Setup(proc));
}

TEST_F(PvAccessReadInstructionTest, missing_channel)
{
  auto procedure = sup::sequencer::ParseProcedureString(PVACCESSMISSINGCHANNELPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  ASSERT_TRUE(procedure->Setup());

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    procedure->ExecuteSingle(&ui);
    exec = procedure->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::FAILURE);
}

PvAccessReadInstructionTest::PvAccessReadInstructionTest()
  : ui{}
{}

PvAccessReadInstructionTest::~PvAccessReadInstructionTest() = default;
