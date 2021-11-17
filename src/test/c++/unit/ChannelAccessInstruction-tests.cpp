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

static ccs::log::Func_t _log_handler = ccs::log::SetStdout();

static const auto ONE_SECOND = 1000000000ul;

static const std::string FETCHBOOLPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <Sequence>
        <ChannelAccessFetchInstruction name="get-client"
            channel="SEQ-TEST:BOOL"
            variable="boolean"/>
        <ChannelAccessFetchInstruction name="get-client"
            channel="SEQ-TEST:BOOL"
            variable="uint32"/>
        <ChannelAccessFetchInstruction name="get-client"
            channel="SEQ-TEST:BOOL"
            variable="string"/>
        <LogTrace input="boolean"/>
        <LogTrace input="uint32"/>
        <LogTrace input="string"/>
    </Sequence>
    <Workspace>
        <FileVariable name="boolean" file="/tmp/file-variable-boolean.dat"/>
        <FileVariable name="uint32" file="/tmp/file-variable-uint32.dat"/>
        <FileVariable name="string" file="/tmp/file-variable-string.dat"/>
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
            <ChannelAccessWriteInstruction name="put-client"
                channel="SEQ-TEST:STRING"
                variable="time"/>
            <ChannelAccessFetchInstruction name="get-client"
                channel="SEQ-TEST:STRING"
                variable="string"/>
            <LogTrace input="time"/>
            <LogTrace input="string"/>
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
            <ChannelAccessWriteInstruction name="put-client"
                channel="SEQ-TEST:STRING"
                variable="time"/>
            <ChannelAccessFetchInstruction name="get-client"
                channel="SEQ-TEST:STRING"
                variable="string"/>
            <LogTrace input="time"/>
            <LogTrace input="string"/>
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

TEST(ChannelAccessInstruction, Execute_missing)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");
  ASSERT_TRUE(static_cast<bool>(instruction));

  sup::sequencer::gtest::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(instruction->GetStatus(), sup::sequencer::ExecutionStatus::FAILURE);
}

TEST(ChannelAccessInstruction, Execute_novar)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("channel", "undefined"));
  EXPECT_TRUE(instruction->AddAttribute("datatype", "{\"type\": \"string\"}"));
  EXPECT_TRUE(instruction->AddAttribute("instance", "\"undefined\""));

  sup::sequencer::gtest::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, nullptr);
  EXPECT_EQ(instruction->GetStatus(), sup::sequencer::ExecutionStatus::FAILURE);
}

TEST_F(ChannelAccessInstructionTest, Fetch_boolean) // Must be associated to a variable in the workspace
{
  sup::sequencer::gtest::NullUserInterface ui;
  auto proc = sup::sequencer::ParseProcedureString(FETCHBOOLPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(proc));

  // Create file to initialise the workspace variables with right datatype
  ccs::types::AnyValue boolean_var (false); ccs::HelperTools::DumpToFile(&boolean_var, "/tmp/file-variable-boolean.dat");
  ccs::types::AnyValue uint32_var (0u); ccs::HelperTools::DumpToFile(&uint32_var, "/tmp/file-variable-uint32.dat");
  ccs::types::AnyValue string_var ("undefined"); ccs::HelperTools::DumpToFile(&string_var, "/tmp/file-variable-string.dat");

  ASSERT_TRUE(proc->Setup());

  (void)::ccs::HelperTools::SleepFor(ONE_SECOND / 2);
  EXPECT_TRUE(::ccs::HelperTools::ExecuteSystemCall("/usr/bin/caput SEQ-TEST:BOOL TRUE"));

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    (void)ccs::HelperTools::SleepFor(ONE_SECOND / 10); // Let system breathe
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);

  // Test variable

  ccs::types::AnyValue val_bool;
  EXPECT_TRUE(::ccs::HelperTools::ReadFromFile(&val_bool, "/tmp/file-variable-boolean.dat"));

  EXPECT_TRUE(static_cast<bool>(val_bool));

   ccs::types::AnyValue val_uint32;
  EXPECT_TRUE(::ccs::HelperTools::ReadFromFile(&val_uint32, "/tmp/file-variable-uint32.dat"));

  EXPECT_EQ(static_cast<ccs::types::uint32>(val_uint32), 1u);

  // Remove temp. files
  (void)::ccs::HelperTools::ExecuteSystemCall("/usr/bin/rm -rf /tmp/file-variable-boolean.dat");
  (void)::ccs::HelperTools::ExecuteSystemCall("/usr/bin/rm -rf /tmp/file-variable-uint32.dat");
  (void)::ccs::HelperTools::ExecuteSystemCall("/usr/bin/rm -rf /tmp/file-variable-string.dat");
}

TEST_F(ChannelAccessInstructionTest, Write_boolean)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:BOOL"));
  EXPECT_TRUE(instruction->AddAttribute("datatype", "{\"type\": \"uint32\"}"));
  EXPECT_TRUE(instruction->AddAttribute("instance", "1"));

  // Setup to verify/process attributes
  sup::sequencer::Procedure proc; // Dummy
  EXPECT_TRUE(instruction->Setup(proc));

  sup::sequencer::gtest::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
  EXPECT_EQ(instruction->GetStatus(), sup::sequencer::ExecutionStatus::SUCCESS);

  // Test variable

  // At this point, the instruction has diconnected form the channel and thread detached from the context
  ASSERT_TRUE(ccs::HelperTools::ChannelAccessClientContext::Attach());

  chid channel;
  (void)ccs::HelperTools::ChannelAccess::ConnectVariable("SEQ-TEST:BOOL", channel);
  (void)ccs::HelperTools::SleepFor(ONE_SECOND / 10);
  ASSERT_TRUE(ccs::HelperTools::ChannelAccess::IsConnected(channel));

  ccs::types::AnyValue value(false);

  EXPECT_TRUE(ccs::HelperTools::ChannelAccess::ReadVariable(
    channel, ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), value.GetInstance()));

  log_info("TEST(ChannelAccessInstruction, Execute_boolean) - Test variable ..");
  EXPECT_TRUE(static_cast<bool>(value));

  (void)ccs::HelperTools::ChannelAccess::DetachVariable(channel);
  (void)ccs::HelperTools::ChannelAccessClientContext::Detach();
}

TEST_F(ChannelAccessInstructionTest, Write_float32)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:FLOAT"));
  EXPECT_TRUE(instruction->AddAttribute("datatype", "{\"type\": \"string\"}"));
  EXPECT_TRUE(instruction->AddAttribute("instance", "\"0.5\""));

  // Setup to verify/process attributes
  sup::sequencer::Procedure proc; // Dummy
  ASSERT_TRUE(instruction->Setup(proc));

  sup::sequencer::gtest::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
  EXPECT_EQ(instruction->GetStatus(), sup::sequencer::ExecutionStatus::SUCCESS);

  // Test variable

  // At this point, the instruction has diconnected form the channel and thread detached from the context
  ASSERT_TRUE(ccs::HelperTools::ChannelAccessClientContext::Attach());

  chid channel;
  (void)ccs::HelperTools::ChannelAccess::ConnectVariable("SEQ-TEST:FLOAT", channel);
  (void)ccs::HelperTools::SleepFor(ONE_SECOND / 10);
  ASSERT_TRUE(ccs::HelperTools::ChannelAccess::IsConnected(channel));

  ccs::types::AnyValue value (static_cast<ccs::types::float32>(0.0));

  EXPECT_TRUE(ccs::HelperTools::ChannelAccess::ReadVariable(channel,
    ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), value.GetInstance()));

  log_info("TEST(ChannelAccessInstruction, Execute_float32) - Test variable ..");
  EXPECT_EQ(static_cast<ccs::types::float32>(value), static_cast<ccs::types::float32>(0.5));

  (void)ccs::HelperTools::ChannelAccess::DetachVariable(channel);
  (void)ccs::HelperTools::ChannelAccessClientContext::Detach();
}

TEST_F(ChannelAccessInstructionTest, Write_array)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("channel", "SEQ-TEST:UIARRAY"));
  EXPECT_TRUE(instruction->AddAttribute("datatype", "{\"type\": \"uint32[8]\",\"multiplicity\":8,\"element\":{\"type\": \"uint32\"}}"));
  EXPECT_TRUE(instruction->AddAttribute("instance", "[1 2 3 4 5 6 7 8]"));

  // Setup to verify/process attributes
  sup::sequencer::Procedure proc; // Dummy
  ASSERT_TRUE(instruction->Setup(proc));

  sup::sequencer::gtest::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
  EXPECT_EQ(instruction->GetStatus(), sup::sequencer::ExecutionStatus::SUCCESS);

  // Test variable

  // At this point, the instruction has diconnected form the channel and thread detached from the context
  ASSERT_TRUE(ccs::HelperTools::ChannelAccessClientContext::Attach());

  chid channel;
  (void)ccs::HelperTools::ChannelAccess::ConnectVariable("SEQ-TEST:UIARRAY", channel);
  (void)ccs::HelperTools::SleepFor(ONE_SECOND / 10);
  ASSERT_TRUE(ccs::HelperTools::ChannelAccess::IsConnected(channel));

  ccs::types::AnyValue value ("{\"type\": \"uint32[8]\",\"multiplicity\":8,\"element\":{\"type\": \"uint32\"}}");

  ASSERT_TRUE(ccs::HelperTools::ChannelAccess::ReadVariable(channel,
    ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), 8u, value.GetInstance()));

  log_info("TEST(ChannelAccessInstruction, Execute_array) - Test variable ..");
  EXPECT_EQ(ccs::HelperTools::GetAttributeValue<ccs::types::uint32>(&value, "[3]"), 4u);

  (void)ccs::HelperTools::ChannelAccess::DetachVariable(channel);
  (void)ccs::HelperTools::ChannelAccessClientContext::Detach();
}

TEST(ChannelAccessInstruction, Write_NoSuchChannel)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("ChannelAccessWriteInstruction");
  ASSERT_TRUE(static_cast<bool>(instruction));

  EXPECT_TRUE(instruction->AddAttribute("channel", "UNDEFINED"));
  EXPECT_TRUE(instruction->AddAttribute("datatype", "{\"type\": \"uint32\"}"));
  EXPECT_TRUE(instruction->AddAttribute("instance", "1"));

  // Setup to verify/process attributes
  sup::sequencer::Procedure proc; // Dummy
  ASSERT_TRUE(instruction->Setup(proc));

  sup::sequencer::gtest::NullUserInterface ui;
  instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
  EXPECT_EQ(instruction->GetStatus(), sup::sequencer::ExecutionStatus::FAILURE); // Expect failure
}

TEST_F(ChannelAccessInstructionTest, ProcedureFile)
{
  std::string file; // Placeholder
  if (::ccs::HelperTools::Exist("../resources/sequence_ca.xml"))
  {
    file = std::string("../resources/sequence_ca.xml");
  }
  else
  {
    file = std::string("./target/test/resources/sequence_ca.xml");
  }

  sup::sequencer::gtest::NullUserInterface ui;
  auto proc = sup::sequencer::ParseProcedureFile(file);

  ASSERT_TRUE(static_cast<bool>(proc));
  ASSERT_TRUE(proc->Setup());
  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    (void)ccs::HelperTools::SleepFor(ONE_SECOND / 100); // Let system breathe
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);
}

TEST_F(ChannelAccessInstructionTest, Procedure_repeat)
{
  sup::sequencer::gtest::NullUserInterface ui;
  auto proc = sup::sequencer::ParseProcedureString(REPEATPROCEDURE);

  ASSERT_TRUE(static_cast<bool>(proc));
  ASSERT_TRUE(proc->Setup());

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    (void)ccs::HelperTools::SleepFor(ONE_SECOND / 100); // Let system breathe
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);
}

// Issue during the tear-down process
TEST_F(ChannelAccessInstructionTest, Procedure_parallel)
{
  sup::sequencer::gtest::NullUserInterface ui; // Should have same or larger scope as procedure
  auto proc = sup::sequencer::ParseProcedureString(PARALLELPROCEDURE);

  ASSERT_TRUE(static_cast<bool>(proc));
  ASSERT_TRUE(proc->Setup());

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    (void)ccs::HelperTools::SleepFor(ONE_SECOND / 100); // Let system breathe
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);
}

ChannelAccessInstructionTest::ChannelAccessInstructionTest() = default;

ChannelAccessInstructionTest::~ChannelAccessInstructionTest() = default;

void ChannelAccessInstructionTest::SetUpTestCase()
{
  if (::ccs::HelperTools::Exist("../resources/ChannelAccessClient.db"))
  {
    ::ccs::HelperTools::ExecuteSystemCall(
        "/usr/bin/screen -d -m -S cainstructiontestIOC /usr/bin/softIoc -d "
        "../resources/ChannelAccessClient.db &> /dev/null");
  }
  else
  {
    ::ccs::HelperTools::ExecuteSystemCall(
        "/usr/bin/screen -d -m -S cainstructiontestIOC /usr/bin/softIoc -d "
        "./target/test/resources/ChannelAccessClient.db &> /dev/null");
  }
  (void)ccs::HelperTools::SleepFor(ONE_SECOND);
}

void ChannelAccessInstructionTest::TearDownTestCase()
{
  ::ccs::HelperTools::ExecuteSystemCall("/usr/bin/screen -S cainstructiontestIOC -X quit &> /dev/null");
}

#undef LOG_ALTERN_SRC
