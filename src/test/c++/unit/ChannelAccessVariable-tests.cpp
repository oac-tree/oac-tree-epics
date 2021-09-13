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

#include <Variable.h>
#include <VariableRegistry.h>

// Local header files

#include "SystemCall.h"

#include "NullUserInterface.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "unit-test"

static const std::string BOOLCONNECTEDTYPE =
  R"RAW({"type":"boolconnected","attributes":[{"value":{"type":"bool"}},{"connected":{"type":"bool"}}]})RAW";


static ccs::log::Func_t _log_handler = ccs::log::SetStdout();

class ChannelAccessVariableTest : public ::testing::Test
{
protected:
  ChannelAccessVariableTest();
  virtual ~ChannelAccessVariableTest();

  bool init_success;
};

// Tests

TEST_F(ChannelAccessVariableTest, GetValue_success)
{
  ASSERT_TRUE(init_success);

  auto variable = sup::sequencer::GlobalVariableRegistry().Create("ChannelAccessVariable");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Setup implicit with AddAttribute .. access as 'string'
  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL") &&
              variable->AddAttribute("datatype", "{\"type\":\"string\"}"));

  ccs::types::AnyValue value; // Placeholder

  ASSERT_TRUE(variable->GetValue(value));
  EXPECT_TRUE(ccs::types::String == value.GetType());

  ccs::types::char8 buffer [1024];
  ccs::HelperTools::SerialiseToJSONStream(&value, buffer, 1024u);
  log_info("TEST(ChannelAccessVariable, GetValue_success) - Value is ..");
  log_info("'%s'", buffer);
}

TEST_F(ChannelAccessVariableTest, GetValue_extended)
{
  ASSERT_TRUE(init_success);

  auto variable = sup::sequencer::GlobalVariableRegistry().Create("ChannelAccessVariable");
  ASSERT_TRUE(static_cast<bool>(variable));

  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL") &&
              variable->AddAttribute("datatype", BOOLCONNECTEDTYPE));

  ccs::types::AnyValue value; // Placeholder

  ASSERT_TRUE(variable->GetValue(value, "value"));
  EXPECT_TRUE(ccs::types::Boolean == value.GetType());

  ccs::types::AnyValue connected; // Placeholder

  ASSERT_TRUE(variable->GetValue(connected, "connected"));
  EXPECT_TRUE(ccs::types::Boolean == connected.GetType());
}

TEST_F(ChannelAccessVariableTest, GetValue_error)
{
  ASSERT_TRUE(init_success);

  auto variable = sup::sequencer::GlobalVariableRegistry().Create("ChannelAccessVariable");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Missing mandatory attribute .. Setup implicit with AddAttribute
  EXPECT_TRUE(variable->AddAttribute("irrelevant", "undefined"));

  ccs::types::AnyValue value; // Placeholder

  EXPECT_FALSE(variable->GetValue(value));
  EXPECT_FALSE(static_cast<bool>(value.GetType()));
}

TEST_F(ChannelAccessVariableTest, SetValue_success)
{
  ASSERT_TRUE(init_success);

  auto variable = sup::sequencer::GlobalVariableRegistry().Create("ChannelAccessVariable");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Setup implicit with AddAttribute .. access as 'float32'
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:FLOAT"));
  EXPECT_TRUE(variable->AddAttribute("datatype", "{\"type\":\"float32\"}"));

  ccs::types::AnyValue value(static_cast<ccs::types::float32>(0.1));

  EXPECT_TRUE(variable->SetValue(value));
  (void)ccs::HelperTools::SleepFor(100000000ul);

  EXPECT_TRUE(variable->GetValue(value));
  ccs::types::float32 val = value;
  EXPECT_FLOAT_EQ(val, 0.1f);

  value = static_cast<ccs::types::float32>(7.5);
  EXPECT_TRUE(variable->SetValue(value));

  (void)ccs::HelperTools::SleepFor(100000000ul);

  EXPECT_TRUE(variable->GetValue(value));
  val = value;
  EXPECT_FLOAT_EQ(val, 7.5f);
}

TEST_F(ChannelAccessVariableTest, ProcedureFile)
{
  ASSERT_TRUE(init_success);
  std::string file; // Placeholder

  if (::ccs::HelperTools::Exist("../resources/variable_ca.xml"))
  {
    file = std::string("../resources/variable_ca.xml");
  }
  else
  {
    file = std::string("./target/test/resources/variable_ca.xml");
  }

  sup::sequencer::gtest::NullUserInterface ui;
  auto proc = sup::sequencer::ParseProcedureFile(file);

  ASSERT_TRUE(static_cast<bool>(proc));

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;
  do
  {
    (void)ccs::HelperTools::SleepFor(100000000ul); // Let system breathe
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));
  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);
}

ChannelAccessVariableTest::ChannelAccessVariableTest()
  : init_success{false}
{
  if (::ccs::HelperTools::Exist("../resources/ChannelAccessClient.db"))
  {
    init_success = ::ccs::HelperTools::ExecuteSystemCall(
        "/usr/bin/screen -d -m -S cavariabletestIOC /usr/bin/softIoc -d ../resources/ChannelAccessClient.db &> /dev/null");
  }
  else
  {
    init_success = ::ccs::HelperTools::ExecuteSystemCall(
        "/usr/bin/screen -d -m -S cavariabletestIOC /usr/bin/softIoc -d ./target/test/resources/ChannelAccessClient.db &> /dev/null");
  }
}

ChannelAccessVariableTest::~ChannelAccessVariableTest()
{
  if (init_success)
  {
    ::ccs::HelperTools::ExecuteSystemCall("/usr/bin/screen -S cavariabletestIOC -X quit &> /dev/null");
  }
}

#undef LOG_ALTERN_SRC
