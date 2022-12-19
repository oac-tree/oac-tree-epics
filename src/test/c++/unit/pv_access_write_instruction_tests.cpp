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

#include <pvxs/pv_access_write_instruction.h>

#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <gtest/gtest.h>

static const std::string PV_ACCESS_READ_SUCCESS_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <RegisterType jsontype='{"type":"seq::pva_write_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <Sequence>
        <Copy input="old-pvxs-value" output="pvxs-variable"/>
        <Equals lhs="pvxs-variable" rhs="old-pvxs-value"/>
        <PvAccessWrite name="write to pv"
            channel="seq::write-test::variable"
            varName="new-pvxs-value"
            timeout="2.0"/>
    </Sequence>
    <Workspace>
        <PvAccessServer name="pvxs-variable"
            channel="seq::write-test::variable"
            type='{"type":"seq::pva_write_test::Type/v1.0"}'/>
        <Local name="old-pvxs-value"
               type='{"type":"seq::pva_write_test::Type/v1.0"}'
               value='{"value":1.0}'/>
        <Local name="new-pvxs-value"
               type='{"type":"seq::pva_write_test::Type/v1.0"}'
               value='{"value":12.5}'/>
    </Workspace>
</Procedure>)RAW";

static const std::string PV_ACCESS_WRONG_OUTPUT_FIELD_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <RegisterType jsontype='{"type":"seq::missing-channel-test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <Sequence>
        <PvAccessWrite name="write to pv"
            channel="seq::test::missing-channel"
            varName="pvxs-value"
            timeout="0.3"/>
    </Sequence>
    <Workspace>
        <Local name="pvxs-value"
               type='{"type":"seq::missing-channel-test::Type/v1.0"}'
               value='{"value":1.0}'/>
    </Workspace>
</Procedure>)RAW";

using namespace sup::sequencer;

class PvAccessWriteInstructionTest : public ::testing::Test
{
protected:
  PvAccessWriteInstructionTest();
  virtual ~PvAccessWriteInstructionTest();

  unit_test_helper::NullUserInterface ui;
};

TEST_F(PvAccessWriteInstructionTest, write_success)
{
  auto procedure = sup::sequencer::ParseProcedureString(PV_ACCESS_READ_SUCCESS_PROCEDURE);
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

    // Creating sequencer's PvAccessClientVariable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", "seq::write-test::variable"));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));

  // reading from the client variable
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue client_val;
    return ws.GetValue("var", client_val) &&
           client_val.HasField("value") &&
           client_val["value"].GetType() == sup::dto::Float32Type &&
           client_val["value"].As<sup::dto::float32>() == 12.5f;
  }));
}

TEST_F(PvAccessWriteInstructionTest, missing_attributes)
{
  sup::sequencer::Procedure proc{};
  proc.AddVariable("SOME_VARIABLE",
    sup::sequencer::GlobalVariableRegistry().Create("Local").release());
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("PvAccessWrite");
  ASSERT_TRUE(static_cast<bool>(instruction));
  EXPECT_FALSE(instruction->Setup(proc));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SOME_CHANNEL"));
  EXPECT_FALSE(instruction->Setup(proc));
  EXPECT_TRUE(instruction->AddAttribute("varName", "SOME_VARIABLE"));
  EXPECT_TRUE(instruction->Setup(proc));
}

TEST_F(PvAccessWriteInstructionTest, missing_variable)
{
  sup::sequencer::Procedure proc{};
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("PvAccessWrite");
  ASSERT_TRUE(static_cast<bool>(instruction));
  EXPECT_FALSE(instruction->Setup(proc));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SOME_CHANNEL"));
  EXPECT_FALSE(instruction->Setup(proc));
  EXPECT_TRUE(instruction->AddAttribute("varName", "SOME_VARIABLE"));
  EXPECT_FALSE(instruction->Setup(proc));
  proc.AddVariable("SOME_VARIABLE",
    sup::sequencer::GlobalVariableRegistry().Create("Local").release());
  EXPECT_TRUE(instruction->Setup(proc));
}

TEST_F(PvAccessWriteInstructionTest, missing_channel)
{
  auto procedure = sup::sequencer::ParseProcedureString(PV_ACCESS_WRONG_OUTPUT_FIELD_PROCEDURE);
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

PvAccessWriteInstructionTest::PvAccessWriteInstructionTest()
  : ui{}
{}

PvAccessWriteInstructionTest::~PvAccessWriteInstructionTest() = default;
