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

#include <sup/sequencer/generic_utils.h>
#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/sequence_parser.h>

#include <cstdlib>

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
        <Log input="boolean"/>
        <Log input="uint32"/>
        <Log input="string"/>
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
            <Wait name="wait" timeout="0.1"/>
            <ChannelAccessWrite name="put-client"
                channel="SEQ-TEST:STRING"
                varName="time"/>
            <ChannelAccessRead name="get-client"
                channel="SEQ-TEST:STRING"
                varName="string"/>
            <Log input="time"/>
            <Log input="string"/>
        </Sequence>
    </Repeat>
    <Workspace>
        <SystemClock name="time" datatype='{"type":"string"}'/>
        <Local name="string" type='{"type":"string"}' value='"undefined"'/>
    </Workspace>
</Procedure>)RAW";

static const std::string PARALLELPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <Repeat maxCount="3">
        <ParallelSequence>
            <Wait name="wait" timeout="0.1"/>
            <ChannelAccessWrite name="put-client"
                channel="SEQ-TEST:STRING"
                varName="time"/>
            <ChannelAccessRead name="get-client"
                channel="SEQ-TEST:STRING"
                varName="string"/>
            <Log input="time"/>
            <Log input="string"/>
        </ParallelSequence>
    </Repeat>
    <Workspace>
        <SystemClock name="time" datatype='{"type":"string"}'/>
        <Local name="string" type='{"type":"string"}' value='"undefined"'/>
    </Workspace>
</Procedure>)RAW";

class ChannelAccessInstructionTest : public ::testing::Test
{
protected:
  ChannelAccessInstructionTest();
  virtual ~ChannelAccessInstructionTest();

  static void SetUpTestCase();
  static void TearDownTestCase();
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
  EXPECT_TRUE(write_instruction->AddAttribute("type", "{\"type\": \"string\"}"));
  EXPECT_TRUE(write_instruction->AddAttribute("value", "\"undefined\""));
  EXPECT_TRUE(write_instruction->Setup(proc));
  write_instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(write_instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(ChannelAccessInstructionTest, DISABLED_ReadBoolean)
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

  // Test variable

  // ccs::types::AnyValue val_bool;
  // EXPECT_TRUE(::ccs::HelperTools::ReadFromFile(&val_bool, "/tmp/file-variable-boolean.dat"));

  // EXPECT_TRUE(static_cast<bool>(val_bool));

  //  ccs::types::AnyValue val_uint32;
  // EXPECT_TRUE(::ccs::HelperTools::ReadFromFile(&val_uint32, "/tmp/file-variable-uint32.dat"));

  // EXPECT_EQ(static_cast<ccs::types::uint32>(val_uint32), 1u);

}

// TEST_F(ChannelAccessInstructionTest, Write_boolean)
// {
//   auto instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
//   ASSERT_TRUE(static_cast<bool>(instruction));

//   EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
//   EXPECT_TRUE(instruction->AddAttribute("datatype", "{\"type\": \"uint32\"}"));
//   EXPECT_TRUE(instruction->AddAttribute("instance", "1"));

//   // Setup to verify/process attributes
//   Procedure proc; // Dummy
//   EXPECT_TRUE(instruction->Setup(proc));

//   gtest::NullUserInterface ui;
//   instruction->ExecuteSingle(&ui, NULL_PTR_CAST(Workspace*));
//   EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::SUCCESS);

//   // Test variable

//   // At this point, the instruction has diconnected form the channel and thread detached from the context
//   ASSERT_TRUE(ccs::HelperTools::ChannelAccessClientContext::Attach());

//   chid channel;
//   (void)ccs::HelperTools::ChannelAccess::ConnectVariable("SEQ-TEST:BOOL", channel);
//   (void)ccs::HelperTools::SleepFor(ONE_SECOND / 10);
//   ASSERT_TRUE(ccs::HelperTools::ChannelAccess::IsConnected(channel));

//   ccs::types::AnyValue value(false);

//   EXPECT_TRUE(ccs::HelperTools::ChannelAccess::ReadVariable(
//     channel, ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), value.GetInstance()));

//   log_info("TEST(ChannelAccessInstruction, Execute_boolean) - Test variable ..");
//   EXPECT_TRUE(static_cast<bool>(value));

//   (void)ccs::HelperTools::ChannelAccess::DetachVariable(channel);
//   (void)ccs::HelperTools::ChannelAccessClientContext::Detach();
// }

// TEST_F(ChannelAccessInstructionTest, Write_float32)
// {
//   auto instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
//   ASSERT_TRUE(static_cast<bool>(instruction));

//   EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:FLOAT"));
//   EXPECT_TRUE(instruction->AddAttribute("datatype", "{\"type\": \"string\"}"));
//   EXPECT_TRUE(instruction->AddAttribute("instance", "\"0.5\""));

//   // Setup to verify/process attributes
//   Procedure proc; // Dummy
//   ASSERT_TRUE(instruction->Setup(proc));

//   gtest::NullUserInterface ui;
//   instruction->ExecuteSingle(&ui, NULL_PTR_CAST(Workspace*));
//   EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::SUCCESS);

//   // Test variable

//   // At this point, the instruction has diconnected form the channel and thread detached from the context
//   ASSERT_TRUE(ccs::HelperTools::ChannelAccessClientContext::Attach());

//   chid channel;
//   (void)ccs::HelperTools::ChannelAccess::ConnectVariable("SEQ-TEST:FLOAT", channel);
//   (void)ccs::HelperTools::SleepFor(ONE_SECOND / 10);
//   ASSERT_TRUE(ccs::HelperTools::ChannelAccess::IsConnected(channel));

//   ccs::types::AnyValue value (static_cast<ccs::types::float32>(0.0));

//   EXPECT_TRUE(ccs::HelperTools::ChannelAccess::ReadVariable(channel,
//     ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), value.GetInstance()));

//   log_info("TEST(ChannelAccessInstruction, Execute_float32) - Test variable ..");
//   EXPECT_EQ(static_cast<ccs::types::float32>(value), static_cast<ccs::types::float32>(0.5));

//   (void)ccs::HelperTools::ChannelAccess::DetachVariable(channel);
//   (void)ccs::HelperTools::ChannelAccessClientContext::Detach();
// }

// TEST_F(ChannelAccessInstructionTest, Write_array)
// {
//   auto instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
//   ASSERT_TRUE(static_cast<bool>(instruction));

//   EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:UIARRAY"));
//   EXPECT_TRUE(instruction->AddAttribute("datatype", "{\"type\": \"uint32[8]\",\"multiplicity\":8,\"element\":{\"type\": \"uint32\"}}"));
//   EXPECT_TRUE(instruction->AddAttribute("instance", "[1 2 3 4 5 6 7 8]"));

//   // Setup to verify/process attributes
//   Procedure proc; // Dummy
//   ASSERT_TRUE(instruction->Setup(proc));

//   gtest::NullUserInterface ui;
//   instruction->ExecuteSingle(&ui, NULL_PTR_CAST(Workspace*));
//   EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::SUCCESS);

//   // Test variable

//   // At this point, the instruction has diconnected form the channel and thread detached from the context
//   ASSERT_TRUE(ccs::HelperTools::ChannelAccessClientContext::Attach());

//   chid channel;
//   (void)ccs::HelperTools::ChannelAccess::ConnectVariable("SEQ-TEST:UIARRAY", channel);
//   (void)ccs::HelperTools::SleepFor(ONE_SECOND / 10);
//   ASSERT_TRUE(ccs::HelperTools::ChannelAccess::IsConnected(channel));

//   ccs::types::AnyValue value ("{\"type\": \"uint32[8]\",\"multiplicity\":8,\"element\":{\"type\": \"uint32\"}}");

//   ASSERT_TRUE(ccs::HelperTools::ChannelAccess::ReadVariable(channel,
//     ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), 8u, value.GetInstance()));

//   log_info("TEST(ChannelAccessInstruction, Execute_array) - Test variable ..");
//   EXPECT_EQ(ccs::HelperTools::GetAttributeValue<ccs::types::uint32>(&value, "[3]"), 4u);

//   (void)ccs::HelperTools::ChannelAccess::DetachVariable(channel);
//   (void)ccs::HelperTools::ChannelAccessClientContext::Detach();
// }

// TEST(ChannelAccessInstruction, Write_NoSuchChannel)
// {
//   auto instruction = GlobalInstructionRegistry().Create("ChannelAccessWrite");
//   ASSERT_TRUE(static_cast<bool>(instruction));

//   EXPECT_TRUE(instruction->AddAttribute("channel", "UNDEFINED"));
//   EXPECT_TRUE(instruction->AddAttribute("datatype", "{\"type\": \"uint32\"}"));
//   EXPECT_TRUE(instruction->AddAttribute("instance", "1"));

//   // Setup to verify/process attributes
//   Procedure proc; // Dummy
//   ASSERT_TRUE(instruction->Setup(proc));

//   gtest::NullUserInterface ui;
//   instruction->ExecuteSingle(&ui, NULL_PTR_CAST(Workspace*));
//   EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::FAILURE); // Expect failure
// }

// TEST_F(ChannelAccessInstructionTest, ProcedureFile)
// {
//   std::string file; // Placeholder
//   if (::ccs::HelperTools::Exist("../resources/sequence_ca.xml"))
//   {
//     file = std::string("../resources/sequence_ca.xml");
//   }
//   else
//   {
//     file = std::string("./target/test/resources/sequence_ca.xml");
//   }

//   gtest::NullUserInterface ui;
//   auto proc = ParseProcedureFile(file);

//   ASSERT_TRUE(static_cast<bool>(proc));
//   ASSERT_TRUE(proc->Setup());
//   ExecutionStatus exec = ExecutionStatus::FAILURE;

//   do
//   {
//     (void)ccs::HelperTools::SleepFor(ONE_SECOND / 100); // Let system breathe
//     proc->ExecuteSingle(&ui);
//     exec = proc->GetStatus();
//   } while ((ExecutionStatus::SUCCESS != exec) &&
//            (ExecutionStatus::FAILURE != exec));

//   EXPECT_EQ(exec, ExecutionStatus::SUCCESS);
// }

// TEST_F(ChannelAccessInstructionTest, Procedure_repeat)
// {
//   gtest::NullUserInterface ui;
//   auto proc = ParseProcedureString(REPEATPROCEDURE);

//   ASSERT_TRUE(static_cast<bool>(proc));
//   ASSERT_TRUE(proc->Setup());

//   ExecutionStatus exec = ExecutionStatus::FAILURE;

//   do
//   {
//     (void)ccs::HelperTools::SleepFor(ONE_SECOND / 100); // Let system breathe
//     proc->ExecuteSingle(&ui);
//     exec = proc->GetStatus();
//   } while ((ExecutionStatus::SUCCESS != exec) &&
//            (ExecutionStatus::FAILURE != exec));

//   EXPECT_EQ(exec, ExecutionStatus::SUCCESS);
// }

// // Issue during the tear-down process
// TEST_F(ChannelAccessInstructionTest, Procedure_parallel)
// {
//   gtest::NullUserInterface ui; // Should have same or larger scope as procedure
//   auto proc = ParseProcedureString(PARALLELPROCEDURE);

//   ASSERT_TRUE(static_cast<bool>(proc));
//   ASSERT_TRUE(proc->Setup());

//   ExecutionStatus exec = ExecutionStatus::FAILURE;

//   do
//   {
//     (void)ccs::HelperTools::SleepFor(ONE_SECOND / 100); // Let system breathe
//     proc->ExecuteSingle(&ui);
//     exec = proc->GetStatus();
//   } while ((ExecutionStatus::SUCCESS != exec) &&
//            (ExecutionStatus::FAILURE != exec));

//   EXPECT_EQ(exec, ExecutionStatus::SUCCESS);
// }

ChannelAccessInstructionTest::ChannelAccessInstructionTest() = default;

ChannelAccessInstructionTest::~ChannelAccessInstructionTest() = default;

void ChannelAccessInstructionTest::SetUpTestCase()
{
  if (utils::FileExists("../resources/ChannelAccessClient.db"))
  {
    std::system(
        "/usr/bin/screen -d -m -S cainstructiontestIOC /usr/bin/softIoc -d "
        "../resources/ChannelAccessClient.db &> /dev/null");
  }
  else
  {
    std::system(
        "/usr/bin/screen -d -m -S cainstructiontestIOC /usr/bin/softIoc -d "
        "./target/test/resources/ChannelAccessClient.db &> /dev/null");
  }
}

void ChannelAccessInstructionTest::TearDownTestCase()
{
  std::system("/usr/bin/screen -S cainstructiontestIOC -X quit &> /dev/null");
}
