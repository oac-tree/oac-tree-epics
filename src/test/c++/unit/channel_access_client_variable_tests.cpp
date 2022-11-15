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
#include "softioc_runner.h"
#include "unit_test_helper.h"

#include <sup/sequencer/variable.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <sup/epics/channel_access_client.h>

#include <gtest/gtest.h>

#include <thread>

static const std::string BOOLCONNECTEDTYPE =
  R"RAW({"type":"boolconnected","attributes":[{"value":{"type":"bool"}},{"connected":{"type":"bool"}}]})RAW";

static const std::string BOOLEXTENDEDTYPE =
  R"RAW({"type":"boolconnected","attributes":[{"value":{"type":"bool"}},{"connected":{"type":"bool"}},{"timestamp":{"type":"uint64"}},{"status":{"type":"int16"}},{"severity":{"type":"int16"}}]})RAW";

static const std::string FLOATTYPE =
  R"RAW({"type":"float32"})RAW";

static const std::string SINGLE_RECORD_DB_CONTENT = R"RAW(
record (bo,"SEQ-TEST:BOOL2")
{
    field(DESC,"Some EPICSv3 record")
    field(ONAM,"TRUE")
    field(OSV,"NO_ALARM")
    field(ZNAM,"FALSE")
    field(ZSV,"NO_ALARM")
    field(VAL,"0")
    field(PINI, "YES")
}
)RAW";

using namespace sup::sequencer;

class ChannelAccessClientVariableTest : public ::testing::Test
{
protected:
  ChannelAccessClientVariableTest();
  ~ChannelAccessClientVariableTest();

  softioc_utils::SoftIocRunner m_softioc_runner;
};

TEST_F(ChannelAccessClientVariableTest, GetValue_success)
{
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
}

TEST_F(ChannelAccessClientVariableTest, GetValue_disconnect)
{
  m_softioc_runner.Start(SINGLE_RECORD_DB_CONTENT);
  ASSERT_TRUE(m_softioc_runner.IsActive());
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL2") &&
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

  // Stop the softIoc
  m_softioc_runner.Stop();
  ASSERT_FALSE(m_softioc_runner.IsActive());
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0, false));

  ASSERT_TRUE(ws.GetValue("var.connected", connected));
  EXPECT_EQ(connected.GetType(), sup::dto::BooleanType);
  EXPECT_FALSE(connected.As<bool>());
}

TEST_F(ChannelAccessClientVariableTest, GetValue_extended)
{
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
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Missing mandatory attribute
  EXPECT_TRUE(variable->AddAttribute("irrelevant", "undefined"));
  EXPECT_NO_THROW(variable->Setup());

  sup::dto::AnyValue value; // Placeholder

  EXPECT_FALSE(variable->GetValue(value));
  EXPECT_TRUE(sup::dto::IsEmptyValue(value));
}

TEST_F(ChannelAccessClientVariableTest, SetValue_success)
{
  Workspace ws;
  sup::epics::ChannelAccessClient ca_client;

  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Setup implicit with AddAttribute .. access as 'float32'
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:FLOAT"));
  EXPECT_TRUE(variable->AddAttribute("type", FLOATTYPE));
  EXPECT_TRUE(ws.AddVariable("var", variable.release()));
  EXPECT_NO_THROW(ws.Setup());


  // Add channel to CA client reader
  EXPECT_TRUE(ca_client.AddVariable("SEQ-TEST:FLOAT", sup::dto::Float32Type));
  EXPECT_TRUE(ca_client.WaitForConnected("SEQ-TEST:FLOAT", 5.0));

  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));

  sup::dto::AnyValue value{0.1f};

  EXPECT_TRUE(ws.SetValue("var", value));

  // Read from variable
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.As<sup::dto::float32>() == 0.1f;
  }));

  // Read from CA client
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ca_client]{
    sup::dto::AnyValue tmp = ca_client.GetValue("SEQ-TEST:FLOAT");
    return !sup::dto::IsEmptyValue(tmp) && tmp.As<sup::dto::float32>() == 0.1f;
  }));

  value = 3.5f;
  EXPECT_TRUE(ws.SetValue("var", value));

  // Read from CA client
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ca_client]{
    sup::dto::AnyValue tmp = ca_client.GetValue("SEQ-TEST:FLOAT");
    return !sup::dto::IsEmptyValue(tmp) && tmp.As<sup::dto::float32>() == 3.5f;
  }));

  // Read from variable
  EXPECT_TRUE(unit_test_helper::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.As<sup::dto::float32>() == 3.5f;
  }));
}

ChannelAccessClientVariableTest::ChannelAccessClientVariableTest()
  : m_softioc_runner{"seq-disconnect-softioc"}
{}

ChannelAccessClientVariableTest::~ChannelAccessClientVariableTest() = default;
