/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP oac-tree
 *
 * Description   : Unit test code
 *
 * Author        : B.Bauvir (IO)
 *
 * Copyright (c) : 2010-2026 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 * SPDX-License-Identifier: MIT
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file LICENSE located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include <sup/epics-test/unit_test_helper.h>
#include "unit_test_helper.h"

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/sequence_parser.h>
#include <sup/oac-tree/user_interface.h>
#include <sup/oac-tree/variable.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>
#include <sup/epics-test/softioc_runner.h>

#include <sup/epics/channel_access_client.h>

#include <gtest/gtest.h>

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

static const std::string LONG_RECORD_DB_CONTENT = R"RAW(
record (longout,"SEQ-TEST:LONG")
{
    field(DESC,"Some EPICSv3 record")
    field(VAL,"4")
    field(PINI,"YES")
}
)RAW";

using namespace sup::oac_tree;

class ChannelAccessClientVariableTest : public ::testing::Test
{
protected:
  ChannelAccessClientVariableTest();
  ~ChannelAccessClientVariableTest();

  sup::epics::test::SoftIocRunner m_softioc_runner;
};

TEST_F(ChannelAccessClientVariableTest, Setup)
{
  Workspace ws;
  // Missing attributes or parse error
  auto var_1 = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(var_1));
  EXPECT_THROW(var_1->Setup(ws), VariableSetupException);
  EXPECT_TRUE(var_1->AddAttribute("channel", "DOESNT-MATTER"));
  EXPECT_THROW(var_1->Setup(ws), VariableSetupException);
  EXPECT_TRUE(var_1->AddAttribute("type", "Cannot parse this as a type"));
  EXPECT_THROW(var_1->Setup(ws), VariableSetupException);

  // Missing attributes or wrong type
  auto var_2 = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(var_2));
  EXPECT_THROW(var_2->Setup(ws), VariableSetupException);
  EXPECT_TRUE(var_2->AddAttribute("channel", "DOESNT-MATTER"));
  EXPECT_THROW(var_2->Setup(ws), VariableSetupException);
  EXPECT_TRUE(var_2->AddAttribute("type", R"RAW({"type":"empty"})RAW"));
  EXPECT_THROW(var_2->Setup(ws), VariableSetupException);

  // Successful setup
  auto var_3 = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(var_3));
  EXPECT_THROW(var_3->Setup(ws), VariableSetupException);
  EXPECT_TRUE(var_3->AddAttribute("channel", "DOESNT-MATTER"));
  EXPECT_THROW(var_3->Setup(ws), VariableSetupException);
  EXPECT_TRUE(var_3->AddAttribute("type", R"RAW({"type":"bool"})RAW"));
  EXPECT_NO_THROW(var_3->Setup(ws));
  EXPECT_NO_THROW(var_3->Teardown());
}

TEST_F(ChannelAccessClientVariableTest, GetValueSuccess)
{
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Access boolean as 'string'
  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL") &&
              variable->AddAttribute("type", "{\"type\":\"string\"}"));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
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

TEST_F(ChannelAccessClientVariableTest, GetValueConnected)
{
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL") &&
              variable->AddAttribute("type", BOOLCONNECTEDTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
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

TEST_F(ChannelAccessClientVariableTest, GetValueDisconnect)
{
  m_softioc_runner.Start(SINGLE_RECORD_DB_CONTENT);
  ASSERT_TRUE(m_softioc_runner.IsActive());
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL2") &&
              variable->AddAttribute("type", BOOLCONNECTEDTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
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

TEST_F(ChannelAccessClientVariableTest, CopyValueDisconnect)
{
  DefaultUserInterface ui;
  std::string procedure_body{
      R"RAW(
  <ParallelSequence isRoot="true" successThreshold="1" failureThreshold="1">
    <ForceSuccess>
      <Wait timeout="10.0"/>
    </ForceSuccess>
    <ForceSuccess>
      <Listen varNames="CA1">
        <Inverter>
          <Sequence>
            <Copy inputVar="CA1" outputVar="var1"/>
            <Equals leftVar="True" rightVar="var1.connected"/>
          </Sequence>
        </Inverter>
      </Listen>
    </ForceSuccess>
  </ParallelSequence>
  <Workspace>
    <ChannelAccessClient channel="SEQ-TEST:LONG" name="CA1" type='{"type":"struct","attributes":[{"connected":{"type":"bool"}},{"value":{"type":"int64"}}]}'/>
    <Local dynamicType="true" name="var1" type='{"type":"bool"}' value="false"/>
    <Local name="True" type='{"type":"bool"}' value="true"/>
    <Local name="False" type='{"type":"bool"}' value="false"/>
  </Workspace>
)RAW"};

  m_softioc_runner.Start(LONG_RECORD_DB_CONTENT);
  // read variable when server is online
  ASSERT_TRUE(m_softioc_runner.IsActive());
  const auto procedure_string_true = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string_true);
  EXPECT_TRUE(unit_test_helper::TryAndExecuteNoReset(proc, ui, ExecutionStatus::SUCCESS));
  Workspace &ws = proc->GetWorkspace();
  sup::dto::AnyValue connected_var;
  EXPECT_TRUE(ws.GetValue("var1", connected_var));
  EXPECT_TRUE(connected_var["connected"].As<bool>());
  proc->Reset(ui);

  // turn off the server
  m_softioc_runner.Stop();

  // change the procedure to expect the connected field to be false
  std::string sub_true = "leftVar=\"True\"";
  std::string sub_false = "leftVar=\"False\"";
  procedure_body.replace(procedure_body.find(sub_true), sub_true.size(), sub_false);
  const auto procedure_string_false = unit_test_helper::CreateProcedureString(procedure_body);
  proc = ParseProcedureString(procedure_string_false);
  EXPECT_TRUE(unit_test_helper::TryAndExecuteNoReset(proc, ui, ExecutionStatus::SUCCESS));
  sup::dto::AnyValue connected_var2;
  Workspace &ws2 = proc->GetWorkspace();
  EXPECT_TRUE(ws2.GetValue("var1", connected_var2));
  EXPECT_FALSE(connected_var2["connected"].As<bool>());
}

TEST_F(ChannelAccessClientVariableTest, GetValueExtended)
{
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  ASSERT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:BOOL") &&
              variable->AddAttribute("type", BOOLEXTENDEDTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
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

TEST_F(ChannelAccessClientVariableTest, GetValueError)
{
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Missing mandatory attribute
  EXPECT_TRUE(variable->AddAttribute("irrelevant", "undefined"));
  EXPECT_THROW(variable->Setup(ws), VariableSetupException);

  sup::dto::AnyValue value;

  EXPECT_FALSE(variable->GetValue(value));
  EXPECT_TRUE(sup::dto::IsEmptyValue(value));
}

TEST_F(ChannelAccessClientVariableTest, SetValueSuccess)
{
  Workspace ws;
  sup::epics::ChannelAccessClient ca_client;

  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Setup implicit with AddAttribute .. access as 'float32'
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:FLOAT"));
  EXPECT_TRUE(variable->AddAttribute("type", FLOATTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());

  // Add channel to CA client reader
  EXPECT_TRUE(ca_client.AddVariable("SEQ-TEST:FLOAT", sup::dto::Float32Type));
  EXPECT_TRUE(ca_client.WaitForConnected("SEQ-TEST:FLOAT", 5.0));

  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));

  // Set using variable
  sup::dto::AnyValue value{0.1f};
  EXPECT_TRUE(ws.SetValue("var", value));
  // Read from variable
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.As<sup::dto::float32>() == 0.1f;
  }));
  // Read from CA client
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ca_client]{
    sup::dto::AnyValue tmp = ca_client.GetValue("SEQ-TEST:FLOAT");
    return !sup::dto::IsEmptyValue(tmp) && tmp.As<sup::dto::float32>() == 0.1f;
  }));

  // Set using CA client
  value = 3.5f;
  EXPECT_TRUE(ca_client.SetValue("SEQ-TEST:FLOAT", value));

  // Read from variable
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue tmp;
    return ws.GetValue("var", tmp) && tmp.As<sup::dto::float32>() == 3.5f;
  }));

  // Read from CA client
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ca_client]{
    sup::dto::AnyValue tmp = ca_client.GetValue("SEQ-TEST:FLOAT");
    return !sup::dto::IsEmptyValue(tmp) && tmp.As<sup::dto::float32>() == 3.5f;
  }));
}

TEST_F(ChannelAccessClientVariableTest, SetValueBadSetup)
{
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Missing mandatory attribute
  EXPECT_TRUE(variable->AddAttribute("irrelevant", "undefined"));
  EXPECT_THROW(variable->Setup(ws), VariableSetupException);

  sup::dto::AnyValue value = 2;

  EXPECT_FALSE(variable->SetValue(value));
}

TEST_F(ChannelAccessClientVariableTest, SetValueUnconnected)
{
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Missing mandatory attribute
  EXPECT_TRUE(variable->AddAttribute("channel", "DOESNOTEXIST"));
  EXPECT_TRUE(variable->AddAttribute("type", FLOATTYPE));
  EXPECT_NO_THROW(variable->Setup(ws));

  sup::dto::AnyValue value = 2;

  EXPECT_FALSE(variable->SetValue(value));
}

TEST_F(ChannelAccessClientVariableTest, SetValueWrongStruct)
{
  Workspace ws;

  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  ASSERT_TRUE(static_cast<bool>(variable));

  // Setup implicit with AddAttribute .. access as 'float32'
  EXPECT_TRUE(variable->AddAttribute("channel", "SEQ-TEST:FLOAT"));
  EXPECT_TRUE(variable->AddAttribute("type", FLOATTYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));

  // Set value using a struct with a 'value' field:
  sup::dto::AnyValue struct_value = {
    {"value", {sup::dto::Float32Type, 22.25f}}
  };
  EXPECT_TRUE(ws.SetValue("var", struct_value));

  sup::dto::AnyValue wrong_struct = {
    {"measurement", {sup::dto::Float32Type, -1.5f}}
  };
  EXPECT_FALSE(ws.SetValue("var", wrong_struct));
}

ChannelAccessClientVariableTest::ChannelAccessClientVariableTest()
  : m_softioc_runner{"seq-disconnect-softioc"}
{}

ChannelAccessClientVariableTest::~ChannelAccessClientVariableTest() = default;
