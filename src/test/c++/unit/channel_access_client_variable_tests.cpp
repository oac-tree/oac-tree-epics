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

#include <sup/sequencer/generic_utils.h>
#include <sup/sequencer/variable.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <gtest/gtest.h>

#include <cstdlib>
#include <thread>

static const std::string BOOLCONNECTEDTYPE =
  R"RAW({"type":"boolconnected","attributes":[{"value":{"type":"bool"}},{"connected":{"type":"bool"}}]})RAW";

static const std::string BOOLEXTENDEDTYPE =
  R"RAW({"type":"boolconnected","attributes":[{"value":{"type":"bool"}},{"connected":{"type":"bool"}},{"timestamp":{"type":"uint64"}},{"status":{"type":"int16"}},{"severity":{"type":"int16"}}]})RAW";

static const std::string FLOATTYPE =
  R"RAW({"type":"float32"})RAW";

using namespace sup::sequencer;

class ChannelAccessClientVariableTest : public ::testing::Test
{
protected:
  ChannelAccessClientVariableTest();
  virtual ~ChannelAccessClientVariableTest();

  void StopIOC();

  bool init_success;
};

TEST_F(ChannelAccessClientVariableTest, GetValue_success)
{
  ASSERT_TRUE(init_success);

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Access boolean as 'string'
  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL") &&
              variable->AddAttribute("type", "{\"type\":\"string\"}"));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  sup::dto::AnyValue value;

  ASSERT_TRUE(ws.GetValue("var", value));
  EXPECT_FALSE(sup::dto::IsEmptyValue(value));
  EXPECT_TRUE(sup::dto::IsScalarValue(value));
  EXPECT_FALSE(sup::dto::IsStructValue(value));
  EXPECT_FALSE(sup::dto::IsArrayValue(value));
  EXPECT_EQ(value.GetType(), sup::dto::StringType);
}

TEST_F(ChannelAccessClientVariableTest, GetValue_connected)
{
  ASSERT_TRUE(init_success);

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL") &&
              variable->AddAttribute("type", BOOLCONNECTEDTYPE));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  sup::dto::AnyValue value;
  ASSERT_TRUE(ws.GetValue("var.value", value));
  EXPECT_EQ(value.GetType(), sup::dto::BooleanType);

  sup::dto::AnyValue connected;
  ASSERT_TRUE(ws.GetValue("var.connected", connected));
  EXPECT_EQ(connected.GetType(), sup::dto::BooleanType);
  EXPECT_TRUE(connected.As<bool>());

  StopIOC();
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0, false));

  ASSERT_TRUE(ws.GetValue("var.connected", connected));
  EXPECT_EQ(connected.GetType(), sup::dto::BooleanType);
  EXPECT_FALSE(connected.As<bool>());
}

TEST_F(ChannelAccessClientVariableTest, GetValue_extended)
{
  ASSERT_TRUE(init_success);

  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL") &&
              variable->AddAttribute("type", BOOLEXTENDEDTYPE));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());

  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));
  sup::dto::AnyValue value;

  ASSERT_TRUE(ws.GetValue("var.value", value));
  EXPECT_EQ(value.GetType(), sup::dto::BooleanType);

  sup::dto::AnyValue connected;
  ASSERT_TRUE(ws.GetValue("var.connected", connected));
  EXPECT_EQ(connected.GetType(), sup::dto::BooleanType);
  EXPECT_TRUE(connected.As<bool>());

  sup::dto::AnyValue timestamp;
  ASSERT_TRUE(ws.GetValue("var.timestamp", timestamp));
  EXPECT_EQ(timestamp.GetType(), sup::dto::UnsignedInteger64Type);
  EXPECT_TRUE(timestamp.As<sup::dto::uint64>() > 0);

  sup::dto::AnyValue status;
  ASSERT_TRUE(ws.GetValue("var.status", status));
  EXPECT_EQ(status.GetType(), sup::dto::SignedInteger16Type);

  sup::dto::AnyValue severity;
  ASSERT_TRUE(ws.GetValue("var.severity", severity));
  EXPECT_EQ(severity.GetType(), sup::dto::SignedInteger16Type);
  EXPECT_EQ(severity.As<sup::dto::int16>(), 0);
}

TEST_F(ChannelAccessClientVariableTest, GetValue_error)
{
  ASSERT_TRUE(init_success);

  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Missing mandatory attribute
  EXPECT_TRUE(variable->AddAttribute("irrelevant", "undefined"));
  EXPECT_NO_THROW(variable->Setup());

  sup::dto::AnyValue value; // Placeholder

  EXPECT_FALSE(variable->GetValue(value));
  EXPECT_TRUE(sup::dto::IsEmptyValue(value));
}

// TEST_F(ChannelAccessClientVariableTest, SetValue_success)
// {
//   ASSERT_TRUE(init_success);
//   ccs::base::ChannelAccessClient ca_reader;

//   auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
//   ASSERT_TRUE(static_cast<bool>(variable));

//   // Setup implicit with AddAttribute .. access as 'float32'
//   EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:FLOAT"));
//   EXPECT_TRUE(variable->AddAttribute("type", FLOATTYPE));
//   EXPECT_NO_THROW(variable->Setup());

//   // Add channel to CA client reader
//   EXPECT_TRUE(ca_reader.AddVariable("SEQ-TEST:FLOAT", sup::dto::AnyputVariable, sup::dto::Float32));
//   EXPECT_TRUE(ca_reader.WaitForConnected(5.0));

//   sup::dto::AnyValue value(static_cast<sup::dto::float32>(0.1));

//   EXPECT_TRUE(variable->SetValue(value));
//   (void)ccs::HelperTools::SleepFor(ONE_SECOND / 2);

//   // Read from variable
//   EXPECT_TRUE(variable->GetValue(value));
//   sup::dto::float32 val = value;
//   EXPECT_FLOAT_EQ(val, 0.1f);

//   // Read from CA client
//   EXPECT_TRUE(ca_reader.GetAnyValue("SEQ-TEST:FLOAT", value));
//   val = value;
//   EXPECT_FLOAT_EQ(val, 0.1f);

//   value = static_cast<sup::dto::float32>(3.5);
//   EXPECT_TRUE(variable->SetValue(value));

//   std::this_thread::sleep_for(std::chrono::milliseconds(500));

//   // Read from CA client
//   EXPECT_TRUE(ca_reader.GetAnyValue("SEQ-TEST:FLOAT", value));
//   val = value;
//   EXPECT_FLOAT_EQ(val, 3.5f);

//   // Read from variable
//   EXPECT_TRUE(variable->GetValue(value));
//   val = value;
//   EXPECT_FLOAT_EQ(val, 3.5f);

//   EXPECT_TRUE(ca_reader.Reset());
// }

// TEST_F(ChannelAccessClientVariableTest, ProcedureFile)
// {
//   ASSERT_TRUE(init_success);
//   std::string file; // Placeholder

//   if (::ccs::HelperTools::Exist("../resources/variable_ca.xml"))
//   {
//     file = std::string("../resources/variable_ca.xml");
//   }
//   else
//   {
//     file = std::string("./target/test/resources/variable_ca.xml");
//   }

//   gtest::NullUserInterface ui;
//   auto proc = ParseProcedureFile(file);

//   ASSERT_TRUE(static_cast<bool>(proc));
//   ASSERT_TRUE(proc->Setup());

//   ExecutionStatus exec = ExecutionStatus::FAILURE;
//   do
//   {
//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     proc->ExecuteSingle(&ui);
//     exec = proc->GetStatus();
//   } while ((ExecutionStatus::SUCCESS != exec) &&
//            (ExecutionStatus::FAILURE != exec));
//   EXPECT_EQ(exec, ExecutionStatus::SUCCESS);
// }

ChannelAccessClientVariableTest::ChannelAccessClientVariableTest()
  : init_success{false}
{
  if (utils::FileExists("../resources/ChannelAccessClient.db"))
  {
    init_success = unit_test_helper::SystemCall(
        "/usr/bin/screen -d -m -S cavariabletestIOC /usr/bin/softIoc -d ../resources/ChannelAccessClient.db &> /dev/null");
  }
  else
  {
    init_success = unit_test_helper::SystemCall(
        "/usr/bin/screen -d -m -S cavariabletestIOC /usr/bin/softIoc -d ./target/test/resources/ChannelAccessClient.db &> /dev/null");
  }
}

ChannelAccessClientVariableTest::~ChannelAccessClientVariableTest()
{
  StopIOC();
}

void ChannelAccessClientVariableTest::StopIOC()
{
  if (init_success)
  {
    unit_test_helper::SystemCall("/usr/bin/screen -S cavariabletestIOC -X quit &> /dev/null");
    init_success = false;
  }
}
