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

#include "null_user_interface.h"
#include "unit_test_helper.h"

#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/variable.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <gtest/gtest.h>

using namespace sup::sequencer;

static const std::string READBOOLPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <Sequence>
        <ChannelAccessRead name="get-client"
            channel="SEQ-TEST:BOOL"
            varName="boolean"/>
        <ChannelAccessRead name="get-client"
            channel="SEQ-TEST:BOOL"
            varName="uint32"/>
        <ChannelAccessRead name="get-client"
            channel="SEQ-TEST:BOOL"
            varName="string"/>
    </Sequence>
    <Workspace>
        <Local name="boolean" type='{"type":"bool"}'/>
        <Local name="uint32" type='{"type":"uint32"}'/>
        <Local name="string" type='{"type":"string"}'/>
    </Workspace>
</Procedure>)RAW";

static const std::string REPEATPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <Repeat maxCount="3">
        <Sequence>
            <Copy input="time" output="tmp"/>
            <ChannelAccessWrite name="put-client"
                channel="SEQ-TEST:STRING"
                varName="tmp"/>
            <ChannelAccessRead name="get-client"
                channel="SEQ-TEST:STRING"
                varName="readback"/>
            <Equals lhs="tmp" rhs="readback"/>
        </Sequence>
    </Repeat>
    <Workspace>
        <SystemClock name="time" format="ISO8601"/>
        <Local name="tmp" type='{"type":"string"}'/>
        <Local name="readback" type='{"type":"string"}'/>
    </Workspace>
</Procedure>)RAW";

class ChannelAccessInstructionTest : public ::testing::Test
{
protected:
  ChannelAccessInstructionTest();
  virtual ~ChannelAccessInstructionTest();
};

TEST_F(ChannelAccessInstructionTest, MissingAttribute)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  auto read_instruction = GlobalInstructionRegistry().Create("ChannelAccessRead");
  ASSERT_TRUE(static_cast<bool>(read_instruction));
  EXPECT_FALSE(read_instruction->Setup(proc));

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_FALSE(write_instruction->Setup(proc));
}

TEST_F(ChannelAccessInstructionTest, WriteToNonExistingChannel)
{
  unit_test_helper::NullUserInterface ui;
  Procedure proc;

  auto write_instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(write_instruction));
  EXPECT_TRUE(write_instruction->AddAttribute("channel", "undefined"));
  EXPECT_TRUE(write_instruction->AddAttribute("timeout", "1.0"));
  EXPECT_TRUE(write_instruction->AddAttribute("type", "{\"type\": \"string\"}"));
  EXPECT_TRUE(write_instruction->AddAttribute("value", "\"undefined\""));
  EXPECT_TRUE(write_instruction->Setup(proc));
  write_instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(ChannelAccessInstructionTest, ReadBoolean)
{
  unit_test_helper::NullUserInterface ui;
  auto proc = ParseProcedureString(READBOOLPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(proc));
  ASSERT_TRUE(proc->Setup());
  EXPECT_TRUE(std::system("/usr/bin/caput SEQ-TEST:BOOL TRUE") == 0);

  ExecutionStatus exec = ExecutionStatus::FAILURE;
  do
  {
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((ExecutionStatus::SUCCESS != exec) && (ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, ExecutionStatus::SUCCESS);

  auto ws = proc->GetWorkspace();

  // test boolean variable
  sup::dto::AnyValue bool_var;
  EXPECT_TRUE(ws->GetValue("boolean", bool_var));
  EXPECT_TRUE(bool_var == true);

  // test boolean variable
  sup::dto::AnyValue uint32_var;
  EXPECT_TRUE(ws->GetValue("uint32", uint32_var));
  EXPECT_TRUE(uint32_var == 1u);

  // test boolean variable
  sup::dto::AnyValue string_var;
  EXPECT_TRUE(ws->GetValue("string", string_var));
  EXPECT_TRUE(string_var == "TRUE");
}

TEST_F(ChannelAccessInstructionTest, Write_boolean)
{
  auto instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(instruction->AddAttribute("type", "{\"type\": \"uint32\"}"));
  EXPECT_TRUE(instruction->AddAttribute("value", "1"));

  unit_test_helper::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::SUCCESS);

  // Test variable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(variable->AddAttribute("type", "{\"type\":\"bool\"}"));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  // Read from variable
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.As<bool>();
  }));
}

TEST_F(ChannelAccessInstructionTest, Write_float32)
{
  auto instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:FLOAT"));
  EXPECT_TRUE(instruction->AddAttribute("type", "{\"type\": \"string\"}"));
  EXPECT_TRUE(instruction->AddAttribute("value", "\"0.5\""));

  unit_test_helper::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::SUCCESS);

  // Test variable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:FLOAT"));
  EXPECT_TRUE(variable->AddAttribute("type", "{\"type\":\"float32\"}"));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  // Read from variable
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.As<sup::dto::float32>() == 0.5f;
  }));
}

TEST_F(ChannelAccessInstructionTest, Write_array)
{
  auto instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:UIARRAY"));
  EXPECT_TRUE(instruction->AddAttribute("type", "{\"type\": \"uint32[8]\",\"multiplicity\":8,\"element\":{\"type\": \"uint32\"}}"));
  EXPECT_TRUE(instruction->AddAttribute("value", "[1, 2, 3, 4, 5, 6, 7, 8]"));

  unit_test_helper::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::SUCCESS);

  // Test variable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:UIARRAY"));
  EXPECT_TRUE(variable->AddAttribute("type", "{\"type\": \"uint32[8]\",\"multiplicity\":8,\"element\":{\"type\": \"uint32\"}}"));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  // Read from variable
  sup::dto::AnyValue readback_val;
  ASSERT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws, &readback_val]{
    return ws.GetValue("var", readback_val) && sup::dto::IsArrayValue(readback_val);
  }));
  EXPECT_TRUE(readback_val[3] == 4);
}

TEST_F(ChannelAccessInstructionTest, Write_NoSuchChannel)
{
  auto instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("channel", "UNDEFINED"));
  EXPECT_TRUE(instruction->AddAttribute("type", "{\"type\": \"uint32\"}"));
  EXPECT_TRUE(instruction->AddAttribute("value", "1"));
  EXPECT_TRUE(instruction->AddAttribute("timeout", "1"));

  Procedure proc;
  EXPECT_TRUE(instruction->Setup(proc));

  unit_test_helper::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(ChannelAccessInstructionTest, Procedure_repeat)
{
  unit_test_helper::NullUserInterface ui;
  auto proc = ParseProcedureString(REPEATPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(proc));
  ASSERT_TRUE(proc->Setup());

  ExecutionStatus exec = ExecutionStatus::FAILURE;
  do
  {
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((ExecutionStatus::SUCCESS != exec) &&
           (ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, ExecutionStatus::SUCCESS);
}

ChannelAccessInstructionTest::ChannelAccessInstructionTest() = default;

ChannelAccessInstructionTest::~ChannelAccessInstructionTest() = default;
