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
 * Copyright (c) : 2010-2021 ITER Organization,
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

#include <Instruction.h>
#include <InstructionRegistry.h>
#include <Variable.h>
#include <VariableRegistry.h>

// Local header files

#include "SystemCall.h"

#include "NullUserInterface.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "unit-test"

static const std::string PVACCESSSERVERPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <RegisterType jsontype='{"type":"seq::pva_write_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <Sequence>
        <Copy input="old-pvxs-value" output="pvxs-variable"/>
        <Equals lhs="pvxs-variable" rhs="old-pvxs-value"/>
        <Wait timeout="2.0"/>
        <PVAccessWriteInstruction name="write to pv"
            channel="seq::test::variable"
            variable="new-pvxs-value"/>
        <Wait timeout="2.0"/>
        <Equals lhs="pvxs-variable" rhs="new-pvxs-value"/>
    </Sequence>
    <Workspace>
        <PVServerVariable name="pvxs-variable"
            channel="seq::test::variable"
            datatype='{"type":"seq::pva_write_test::Type/v1.0"}'/>
        <Local name="old-pvxs-value"
               type='{"type":"seq::pva_write_test::Type/v1.0"}'
               value='{"value":1.0}'/>
        <Local name="new-pvxs-value"
               type='{"type":"seq::pva_write_test::Type/v1.0"}'
               value='{"value":12.5}'/>
    </Workspace>
</Procedure>)RAW";

static const std::string PVACCESSMISSINGCHANNELPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <RegisterType jsontype='{"type":"seq::missing-channel-test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <Sequence>
        <PVAccessWriteInstruction name="write to pv"
            channel="seq::test::missing-channel"
            variable="pvxs-value"/>
    </Sequence>
    <Workspace>
        <Local name="pvxs-value"
               type='{"type":"seq::missing-channel-test::Type/v1.0"}'
               value='{"value":1.0}'/>
    </Workspace>
</Procedure>)RAW";

static ccs::log::Func_t _log_handler = ccs::log::SetStdout();

class PVAccessWriteInstructionTest : public ::testing::Test
{
protected:
  PVAccessWriteInstructionTest();
  virtual ~PVAccessWriteInstructionTest();

  sup::sequencer::gtest::NullUserInterface ui;
  sup::sequencer::Procedure proc;
};

// Tests

TEST_F(PVAccessWriteInstructionTest, write_success)
{
  auto procedure = sup::sequencer::ParseProcedureString(PVACCESSSERVERPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  ASSERT_TRUE(procedure->Setup());

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    (void)ccs::HelperTools::SleepFor(100000000ul); // Let system breathe
    procedure->ExecuteSingle(&ui);
    exec = procedure->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);
}

TEST_F(PVAccessWriteInstructionTest, missing_attributes)
{
  proc.AddVariable("SOME_VARIABLE",
    sup::sequencer::GlobalVariableRegistry().Create("Local").release());
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("PVAccessWriteInstruction");
  ASSERT_TRUE(static_cast<bool>(instruction));
  EXPECT_FALSE(instruction->Setup(proc));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SOME_CHANNEL"));
  EXPECT_FALSE(instruction->Setup(proc));
  EXPECT_TRUE(instruction->AddAttribute("variable", "SOME_VARIABLE"));
  EXPECT_TRUE(instruction->Setup(proc));
}

TEST_F(PVAccessWriteInstructionTest, missing_variable)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("PVAccessWriteInstruction");
  ASSERT_TRUE(static_cast<bool>(instruction));
  EXPECT_FALSE(instruction->Setup(proc));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SOME_CHANNEL"));
  EXPECT_FALSE(instruction->Setup(proc));
  EXPECT_TRUE(instruction->AddAttribute("variable", "SOME_VARIABLE"));
  EXPECT_FALSE(instruction->Setup(proc));
  proc.AddVariable("SOME_VARIABLE",
    sup::sequencer::GlobalVariableRegistry().Create("Local").release());
  EXPECT_TRUE(instruction->Setup(proc));
}

TEST_F(PVAccessWriteInstructionTest, missing_channel)
{
  auto procedure = sup::sequencer::ParseProcedureString(PVACCESSMISSINGCHANNELPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  ASSERT_TRUE(procedure->Setup());

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    (void)ccs::HelperTools::SleepFor(100000000ul); // Let system breathe
    procedure->ExecuteSingle(&ui);
    exec = procedure->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::FAILURE);
}

PVAccessWriteInstructionTest::PVAccessWriteInstructionTest()
  : ui{}
  , proc{}
{}

PVAccessWriteInstructionTest::~PVAccessWriteInstructionTest() = default;

#undef LOG_ALTERN_SRC
