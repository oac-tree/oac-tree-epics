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

#include "test_user_interface.h"
#include "unit_test_helper.h"

#include <oac-tree/pvxs/pv_access_read_instruction.h>

#include <sup/epics/pv_access_server.h>
#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/instruction.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/log_severity.h>
#include <sup/oac-tree/procedure.h>
#include <sup/oac-tree/sequence_parser.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <gtest/gtest.h>

static const std::string PV_ACCESS_WRONG_OUTPUT_FIELD_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/oac-tree" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/oac-tree oac-tree.xsd">
    <RegisterType jsontype='{"type":"seq::wrong-field-test::channel-type","attributes":[{"value":{"type":"uint16"}}]}'/>
    <PvAccessRead name="read from pv"
                  channel="pva-read-instr-test::variable2"
                  outputVar="pvxs-value.value"
                  timeout="2.0"/>
    <Workspace>
        <PvAccessServer name="pvxs-variable"
                        channel="pva-read-instr-test::variable2"
                        type='{"type":"seq::wrong-field-test::channel-type"}'/>
                        value='{"value":1.0}'/>
        <Local name="pvxs-value"
               type='{"type":"seq::wrong-field-test::channel-type"}'/>
    </Workspace>
</Procedure>)RAW";

static const std::string PV_ACCESS_READ_SUCCESS_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/oac-tree" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/oac-tree oac-tree.xsd">
    <RegisterType jsontype='{"type":"seq::pva_read_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
    <Sequence>
        <PvAccessRead name="read from pv"
                      channel="pva-read-instr-test::variable"
                      outputVar="pvxs-value"
                      timeout="2.0"/>
        <Equals leftVar="pvxs-variable" rightVar="pvxs-value"/>
    </Sequence>
    <Workspace>
        <PvAccessServer name="pvxs-variable"
                        channel="pva-read-instr-test::variable"
                        type='{"type":"seq::pva_read_test::Type/v1.0"}'/>
                        value='{"value":1.0}'/>
        <Local name="pvxs-value"
               type='{"type":"seq::pva_read_test::Type/v1.0"}'
               value='{"value":0.0}'/>
    </Workspace>
</Procedure>)RAW";

using namespace sup::oac_tree;

class PvAccessReadInstructionTest : public ::testing::Test
{
protected:
  PvAccessReadInstructionTest();
  virtual ~PvAccessReadInstructionTest();

  unit_test_helper::LogUserInterface ui;

  static void SetUpTestCase();
  static void TearDownTestCase();

  static std::unique_ptr<sup::epics::PvAccessServer> server;
};

std::unique_ptr<sup::epics::PvAccessServer> PvAccessReadInstructionTest::server{};

TEST_F(PvAccessReadInstructionTest, Setup)
{
  Procedure proc;
  // read instruction requires a channel and an output attribute
  {
    PvAccessReadInstruction instruction{};
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("outputVar", "Some_Var_Name"));
    EXPECT_NO_THROW(instruction.Setup(proc));
    EXPECT_NO_THROW(instruction.Reset(ui));
  }
  // timeout attribute should be parsed to double
  {
    PvAccessReadInstruction instruction{};
    EXPECT_TRUE(instruction.AddAttribute("channel", "Does_Not_Matter"));
    EXPECT_TRUE(instruction.AddAttribute("outputVar", "Some_Var_Name"));
    EXPECT_TRUE(instruction.AddAttribute("timeout", "1s"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_NO_THROW(instruction.Reset(ui));
  }
}

TEST_F(PvAccessReadInstructionTest, MissingVariable)
{

  Procedure proc;
  Workspace ws;

  PvAccessReadInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "seq-plugin-epics-test::missing-var"));
  EXPECT_TRUE(instruction.AddAttribute("outputVar", "DoesNotExist"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  while (!IsFinishedStatus(instruction.GetStatus()))
  {
    EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  }
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_ERR);
  EXPECT_NE(message.find("DoesNotExist"), std::string::npos);
}

TEST_F(PvAccessReadInstructionTest, Timeout)
{
  Procedure proc;
  Workspace ws;

  auto variable = GlobalVariableRegistry().Create("Local");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));

  PvAccessReadInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("channel", "ThisTimeouts"));
  EXPECT_TRUE(instruction.AddAttribute("outputVar", "var"));
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
  EXPECT_NE(message.find("ThisTimeouts"), std::string::npos);
}

TEST_F(PvAccessReadInstructionTest, ProcedureWrongOutputField)
{
  EXPECT_EQ(ui.m_log_entries.size(), 0);
  auto procedure = ParseProcedureString(PV_ACCESS_WRONG_OUTPUT_FIELD_PROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  EXPECT_NO_THROW(procedure->Setup());

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
  EXPECT_NE(message.find("pvxs-value.value"), std::string::npos);
}

TEST_F(PvAccessReadInstructionTest, ProcedureSuccess)
{
  auto procedure = ParseProcedureString(PV_ACCESS_READ_SUCCESS_PROCEDURE);
  ASSERT_TRUE(static_cast<bool>(procedure));
  EXPECT_NO_THROW(procedure->Setup());

  ExecutionStatus exec = ExecutionStatus::FAILURE;
  do
  {
    procedure->ExecuteSingle(ui);
    exec = procedure->GetStatus();
  } while ((ExecutionStatus::SUCCESS != exec) && (ExecutionStatus::FAILURE != exec));
  EXPECT_EQ(exec, ExecutionStatus::SUCCESS);
}

TEST_F(PvAccessReadInstructionTest, VariableAttributes)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <RegisterType jsontype='{"type":"seq::pva_read_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
  <PvAccessRead channel="@chan" outputVar="pvxs-value" timeout="@mytimeout"/>
  <Workspace>
    <PvAccessServer name="pvxs-variable"
                    channel="pva-read-instr-test::variable3"
                    type='{"type":"seq::pva_read_test::Type/v1.0"}'/>
                    value='{"value":1.0}'/>
    <Local name="pvxs-value"
           type='{"type":"seq::pva_read_test::Type/v1.0"}'
           value='{"value":0.0}'/>
    <Local name="chan" type='{"type":"string"}' value='"pva-read-instr-test::variable3"'/>
    <Local name="mytimeout" type='{"type":"float64"}' value='3.0'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui));
}

TEST_F(PvAccessReadInstructionTest, VariableAttributesWrongType)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <RegisterType jsontype='{"type":"seq::pva_read_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
  <PvAccessRead channel="@chan" outputVar="pvxs-value" timeout="@mytimeout"/>
  <Workspace>
    <PvAccessServer name="pvxs-variable"
                    channel="pva-read-instr-test::variable4"
                    type='{"type":"seq::pva_read_test::Type/v1.0"}'/>
                    value='{"value":1.0}'/>
    <Local name="pvxs-value"
           type='{"type":"seq::pva_read_test::Type/v1.0"}'
           value='{"value":0.0}'/>
    <Local name="chan" type='{"type":"float32"}' value='4.3'/>
    <Local name="mytimeout" type='{"type":"float64"}' value='3.0'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}

TEST_F(PvAccessReadInstructionTest, VariableAttributesNotPresent)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <RegisterType jsontype='{"type":"seq::pva_read_test::Type/v1.0","attributes":[{"value":{"type":"float32"}}]}'/>
  <PvAccessRead channel="@chan" outputVar="pvxs-value" timeout="@mytimeout"/>
  <Workspace>
    <PvAccessServer name="pvxs-variable"
                    channel="pva-read-instr-test::variable5"
                    type='{"type":"seq::pva_read_test::Type/v1.0"}'/>
                    value='{"value":1.0}'/>
    <Local name="pvxs-value"
           type='{"type":"seq::pva_read_test::Type/v1.0"}'
           value='{"value":0.0}'/>
    <Local name="mytimeout" type='{"type":"float64"}' value='3.0'/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}

PvAccessReadInstructionTest::PvAccessReadInstructionTest()
  : ui{}
{
}

PvAccessReadInstructionTest::~PvAccessReadInstructionTest() = default;

void PvAccessReadInstructionTest::SetUpTestCase()
{
  server = std::make_unique<sup::epics::PvAccessServer>();
  sup::dto::AnyValue value = {
    { "enabled", { sup::dto::BooleanType, true }}
  };
  server->AddVariable("seq-plugin-epics-test::missing-var", value);
  server->Start();
}

void PvAccessReadInstructionTest::TearDownTestCase()
{
  server.reset();
}
