/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP oac-tree
 *
 * Description   : Unit test code
 *
 * Author        : Walter Van Herck (IO)
 *
 * Copyright (c) : 2010-2025 ITER Organization,
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

#include "test_user_interface.h"
#include "unit_test_helper.h"

#include <oac-tree/pvxs/pv_access_write_instruction.h>

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/instruction.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/log_severity.h>
#include <sup/oac-tree/procedure.h>
#include <sup/oac-tree/sequence_parser.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <gtest/gtest.h>
#include <sup/epics-test/unit_test_helper.h>

static const std::string UINT16_STRUCT_TYPE =
  R"RAW({"type":"seq-test::uint16-struct-type","attributes":[{"value":{"type":"uint16"}}]})RAW";

static const std::string UINT16_STRUCT_VALUE = R"RAW({"value":42})RAW";

static const std::string PV_ACCESS_CHANNEL_MISMATCH_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/oac-tree" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/oac-tree oac-tree.xsd">
    <RegisterType jsontype='{"type":"seq::pva_write_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <PvAccessWrite name="write to pv"
                   channel="seq::write-test::var_1"
                   varName="pvxs-value"/>
    <Workspace>
        <PvAccessServer name="pvxs-variable"
                        channel="seq::write-test::var_1"
                        type='{"type":"seq::pva_write_test::Type/v1.0"}'/>
        <Local name="pvxs-value"
               type='{"type":"string"}'
               value='"my_string"'/>
    </Workspace>
</Procedure>)RAW";

static const std::string PV_ACCESS_WRITE_SUCCESS_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/oac-tree" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/oac-tree oac-tree.xsd">
    <RegisterType jsontype='{"type":"seq::pva_write_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <PvAccessWrite name="write to pv"
                   channel="seq::write-test::var_2"
                   varName="pvxs-value"/>
    <Workspace>
        <PvAccessServer name="pvxs-variable"
                        channel="seq::write-test::var_2"
                        type='{"type":"seq::pva_write_test::Type/v1.0"}'/>
        <Local name="pvxs-value"
               type='{"type":"seq::pva_write_test::Type/v1.0"}'
               value='{"value":1.0}'/>
    </Workspace>
</Procedure>)RAW";

using namespace sup::oac_tree;

class PvAccessWriteInstructionTest : public ::testing::Test
{
protected:
  PvAccessWriteInstructionTest();
  virtual ~PvAccessWriteInstructionTest();

  unit_test_helper::LogUserInterface ui;
};

TEST_F(PvAccessWriteInstructionTest, Setup)
{
  Procedure proc;
  // write instruction can be setup with channel and varName attribute
  {
    PvAccessWriteInstruction instruction{};
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("varName", "Some_Var_Name"));
    EXPECT_NO_THROW(instruction.Setup(proc));
    EXPECT_NO_THROW(instruction.Reset(ui));
  }
  // write instruction can be setup with channel, type and value attribute
  {
    PvAccessWriteInstruction instruction{};
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("type", "Some_Type"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("value", "Some_Value"));
    EXPECT_NO_THROW(instruction.Setup(proc));
    EXPECT_NO_THROW(instruction.Reset(ui));
  }
  // write instruction needs to be able to parse the timeout attribute as a double
  {
    PvAccessWriteInstruction instruction{};
    EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
    EXPECT_TRUE(instruction.AddAttribute("varName", "Some_Var_Name"));
    EXPECT_TRUE(instruction.AddAttribute("timeout", "Three"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
  }
  // write instruction correctly parses the timeout attribute as a positive double
  {
    PvAccessWriteInstruction instruction{};
    EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
    EXPECT_TRUE(instruction.AddAttribute("varName", "Some_Var_Name"));
    EXPECT_TRUE(instruction.AddAttribute("timeout", "5.0"));
    EXPECT_NO_THROW(instruction.Setup(proc));
    EXPECT_NO_THROW(instruction.Reset(ui));
  }
}

TEST_F(PvAccessWriteInstructionTest, MissingVariable)
{
  Procedure proc;
  Workspace ws;

  PvAccessWriteInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("varName", "DoesNotExist"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_ERR);
  EXPECT_NE(message.find("DoesNotExist"), std::string::npos);
}

TEST_F(PvAccessWriteInstructionTest, MissingVariableField)
{
  Procedure proc;
  Workspace ws;

  auto variable = GlobalVariableRegistry().Create("Local");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("type", UINT16_STRUCT_TYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));

  PvAccessWriteInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("varName", "var.val"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_WARNING);
  EXPECT_NE(message.find("var.val"), std::string::npos);
}

TEST_F(PvAccessWriteInstructionTest, EmptyVariable)
{
  Procedure proc;
  Workspace ws;

  auto variable = std::make_unique<unit_test_helper::ReadOnlyVariable>(sup::dto::AnyValue{});
  EXPECT_NO_THROW(variable->Setup(ws));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));

  PvAccessWriteInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("varName", "var"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_WARNING);
  EXPECT_NE(message.find("var"), std::string::npos);
}

TEST_F(PvAccessWriteInstructionTest, TypeParseError)
{
  Procedure proc;
  Workspace ws;

  PvAccessWriteInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("type", "TypeCannotBeParsed"));
  EXPECT_TRUE(instruction.AddAttribute("value", "ignored"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_ERR);
  EXPECT_NE(message.find("TypeCannotBeParsed"), std::string::npos);
}

TEST_F(PvAccessWriteInstructionTest, ValueParseError)
{
  Procedure proc;
  Workspace ws;

  PvAccessWriteInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("type", UINT16_STRUCT_TYPE));
  EXPECT_TRUE(instruction.AddAttribute("value", "ValueCannotBeParsed"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_ERR);
  EXPECT_NE(message.find("ValueCannotBeParsed"), std::string::npos);
}

TEST_F(PvAccessWriteInstructionTest, ChannelTimeout)
{
  Procedure proc;
  Workspace ws;

  PvAccessWriteInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Exist"));
  EXPECT_TRUE(instruction.AddAttribute("type", UINT16_STRUCT_TYPE));
  EXPECT_TRUE(instruction.AddAttribute("value", UINT16_STRUCT_VALUE));
  EXPECT_TRUE(instruction.AddAttribute("timeout", "0.1"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  while (!IsFinishedStatus(instruction.GetStatus()))
  {
    EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  }
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_WARNING);
  EXPECT_NE(message.find("Does_Not_Exist"), std::string::npos);
}

TEST_F(PvAccessWriteInstructionTest, DISABLED_ChannelTypeMismatch)
{
  auto procedure = ParseProcedureString(PV_ACCESS_CHANNEL_MISMATCH_PROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  EXPECT_NO_THROW(procedure->Setup());
  EXPECT_EQ(ui.m_log_entries.size(), 0);

  ExecutionStatus exec = ExecutionStatus::FAILURE;
  do
  {
    procedure->ExecuteSingle(ui);
    exec = procedure->GetStatus();
  } while ((ExecutionStatus::SUCCESS != exec) &&
           (ExecutionStatus::FAILURE != exec));
  EXPECT_EQ(exec, ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_WARNING);
  EXPECT_NE(message.find("seq::write-test::var_1"), std::string::npos);
  EXPECT_NE(message.find("my_string"), std::string::npos);
}

TEST_F(PvAccessWriteInstructionTest, Success)
{
  auto procedure = ParseProcedureString(PV_ACCESS_WRITE_SUCCESS_PROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  EXPECT_NO_THROW(procedure->Setup());

  ExecutionStatus exec = ExecutionStatus::FAILURE;
  do
  {
    procedure->ExecuteSingle(ui);
    exec = procedure->GetStatus();
  } while ((ExecutionStatus::SUCCESS != exec) &&
           (ExecutionStatus::FAILURE != exec));
  EXPECT_EQ(exec, ExecutionStatus::SUCCESS);

    // Creating oac-tree's PvAccessClientVariable
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("PvAccessClient");
  EXPECT_NO_THROW(variable->AddAttribute("channel", "seq::write-test::var_2"));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));
  EXPECT_NO_THROW(ws.Setup());
  EXPECT_TRUE(ws.WaitForVariable("var", 5.0));

  // reading from the client variable
  EXPECT_TRUE(sup::epics::test::BusyWaitFor(2.0, [&ws]{
    sup::dto::AnyValue client_val;
    return ws.GetValue("var", client_val) &&
           client_val.HasField("value") &&
           client_val["value"].GetType() == sup::dto::Float32Type &&
           client_val["value"].As<sup::dto::float32>() == 1.0f;
  }));
}

TEST_F(PvAccessWriteInstructionTest, VariableAttributes)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <RegisterType jsontype='{"type":"seq::pva_write_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
  <PvAccessWrite channel="@chan" varName="pvxs-value" timeout="@mytimeout"/>
  <Workspace>
    <PvAccessServer name="pvxs-variable"
                    channel="seq::write-test::var_3"
                    type='{"type":"seq::pva_write_test::Type/v1.0"}'/>
    <Local name="pvxs-value"
           type='{"type":"seq::pva_write_test::Type/v1.0"}'
           value='{"value":1.0}'/>
    <Local name="chan" type='{"type":"string"}' value='"seq::write-test::var_3"'/>
    <Local name="mytimeout" type='{"type":"float64"}' value='10.0'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui));
}

TEST_F(PvAccessWriteInstructionTest, VariableAttributesWrongType)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <RegisterType jsontype='{"type":"seq::pva_write_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
  <PvAccessWrite channel="@chan" varName="pvxs-value" timeout="@mytimeout"/>
  <Workspace>
    <PvAccessServer name="pvxs-variable"
                    channel="seq::write-test::var_4"
                    type='{"type":"seq::pva_write_test::Type/v1.0"}'/>
    <Local name="pvxs-value"
           type='{"type":"seq::pva_write_test::Type/v1.0"}'
           value='{"value":1.0}'/>
    <Local name="chan" type='{"type":"string"}' value='"seq::write-test::var_4"'/>
    <Local name="mytimeout" type='{"type":"string"}' value='"3.0"'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}

TEST_F(PvAccessWriteInstructionTest, VariableAttributesNotPresent)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <RegisterType jsontype='{"type":"seq::pva_write_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
  <PvAccessWrite channel="@chan" varName="pvxs-value" timeout="@mytimeout"/>
  <Workspace>
    <PvAccessServer name="pvxs-variable"
                    channel="seq::write-test::var_5"
                    type='{"type":"seq::pva_write_test::Type/v1.0"}'/>
    <Local name="pvxs-value"
           type='{"type":"seq::pva_write_test::Type/v1.0"}'
           value='{"value":1.0}'/>
    <Local name="chan" type='{"type":"string"}' value='"seq::write-test::var_5"'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}

PvAccessWriteInstructionTest::PvAccessWriteInstructionTest()
  : ui{}
{}

PvAccessWriteInstructionTest::~PvAccessWriteInstructionTest() = default;
