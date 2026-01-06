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

#include <oac-tree/pvxs/rpc_client_instruction.h>

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/instruction.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/log_severity.h>
#include <sup/oac-tree/sequence_parser.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <sup/epics/pv_access_rpc_server.h>

#include <sup/protocol/protocol_rpc.h>

#include <gtest/gtest.h>

#include <memory>

static const std::string REQUEST_TYPE =
  R"RAW({"type":"sup::RPCRequest/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"query":{"type":"uint16"}}]})RAW";

static const std::string REQUEST_VALUE = R"RAW({"timestamp":0,"query":42})RAW";

static const std::string TEST_SERVICE_NAME = "RPCClientInstructionTest::service";

static const std::string RPC_CLIENT_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/oac-tree" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/oac-tree oac-tree.xsd">
    <Sequence>
        <RPCClient name="rpc-client"
                   service="RPCClientInstructionTest::service"
                   requestVar="request"
                   outputVar="reply"/>
    </Sequence>
    <Workspace>
        <Local name="request"
               type='{"type":"sup::RPCRequest/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"query":{"type":"bool"}}]}'
               value='{"timestamp":0,"query":true}'/>
        <Local name="reply"/>
    </Workspace>
</Procedure>)RAW";


using namespace sup::oac_tree;

class RPCTestHandler : public sup::dto::AnyFunctor
{
public:
  RPCTestHandler() = default;
  ~RPCTestHandler() = default;

  sup::dto::AnyValue operator()(const sup::dto::AnyValue& input) override
  {
    return sup::protocol::utils::CreateRPCReply(sup::protocol::Success, input["query"],
                                                sup::protocol::PayloadEncoding::kNone);
  }
};

class RPCClientInstructionTest : public ::testing::Test
{
protected:
  RPCClientInstructionTest();
  ~RPCClientInstructionTest();

  static void SetUpTestCase();
  static void TearDownTestCase();

  static RPCTestHandler m_handler;
  static std::unique_ptr<sup::epics::PvAccessRPCServer> server;

  unit_test_helper::LogUserInterface ui;
};

RPCTestHandler RPCClientInstructionTest::m_handler{};
std::unique_ptr<sup::epics::PvAccessRPCServer> RPCClientInstructionTest::server{};

TEST_F(RPCClientInstructionTest, Setup)
{
  Procedure proc;
  // rpc client instruction requires a channel and a request variable attribute
  {
    RPCClientInstruction instruction{};
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Matter"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("requestVar", "Some_Var_Name"));
    EXPECT_NO_THROW(instruction.Setup(proc));
    EXPECT_NO_THROW(instruction.Reset(ui));
  }
  // or a channel and a request type/value attribute
  {
    RPCClientInstruction instruction{};
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Matter"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("type", "Some_Type"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
    EXPECT_TRUE(instruction.AddAttribute("value", "Some_Value"));
    EXPECT_NO_THROW(instruction.Setup(proc));
    EXPECT_NO_THROW(instruction.Reset(ui));
  }
  // rpc client instruction timeout attribute cannot be parsed
  {
    RPCClientInstruction instruction{};
    EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Matter"));
    EXPECT_TRUE(instruction.AddAttribute("requestVar", "Some_Var_Name"));
    EXPECT_TRUE(instruction.AddAttribute("timeout", "CannotBeParsed"));
    EXPECT_THROW(instruction.Setup(proc), InstructionSetupException);
  }
  // rpc client instruction timeout attribute correctly parsed
  {
    RPCClientInstruction instruction{};
    EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Matter"));
    EXPECT_TRUE(instruction.AddAttribute("requestVar", "Some_Var_Name"));
    EXPECT_TRUE(instruction.AddAttribute("timeout", "3.0"));
    EXPECT_NO_THROW(instruction.Setup(proc));
    EXPECT_NO_THROW(instruction.Reset(ui));
  }
}

TEST_F(RPCClientInstructionTest, MissingVariable)
{
  Procedure proc;
  Workspace ws;

  RPCClientInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("requestVar", "DoesNotExist"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_ERR);
  EXPECT_NE(message.find("DoesNotExist"), std::string::npos);
}

TEST_F(RPCClientInstructionTest, MissingVariableField)
{
  Procedure proc;
  Workspace ws;

  auto variable = GlobalVariableRegistry().Create("Local");
  ASSERT_TRUE(static_cast<bool>(variable));
  EXPECT_TRUE(variable->AddAttribute("type", REQUEST_TYPE));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));

  RPCClientInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("requestVar", "var.val"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_WARNING);
  EXPECT_NE(message.find("var.val"), std::string::npos);
}

TEST_F(RPCClientInstructionTest, EmptyVariable)
{
  Procedure proc;
  Workspace ws;

  auto variable = std::make_unique<unit_test_helper::ReadOnlyVariable>(sup::dto::AnyValue{});
  EXPECT_NO_THROW(variable->Setup(ws));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));

  RPCClientInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("requestVar", "var"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
  ASSERT_EQ(ui.m_log_entries.size(), 1);
  auto [severity, message] = ui.m_log_entries.back();
  EXPECT_EQ(severity, log::SUP_SEQ_LOG_WARNING);
  EXPECT_NE(message.find("var"), std::string::npos);
}

TEST_F(RPCClientInstructionTest, TypeParseError)
{
  Procedure proc;
  Workspace ws;

  RPCClientInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Matter"));
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

TEST_F(RPCClientInstructionTest, ValueParseError)
{
  Procedure proc;
  Workspace ws;

  RPCClientInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Matter"));
  EXPECT_TRUE(instruction.AddAttribute("type", REQUEST_TYPE));
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

TEST_F(RPCClientInstructionTest, NegativeTimeout)
{
  Procedure proc;
  Workspace ws;

  RPCClientInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Exist"));
  EXPECT_TRUE(instruction.AddAttribute("type", REQUEST_TYPE));
  EXPECT_TRUE(instruction.AddAttribute("value", REQUEST_VALUE));
  EXPECT_TRUE(instruction.AddAttribute("timeout", "-1.0"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
}


TEST_F(RPCClientInstructionTest, ServiceTimeout)
{
  Procedure proc;
  Workspace ws;

  RPCClientInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("service", "Does_Not_Exist"));
  EXPECT_TRUE(instruction.AddAttribute("type", REQUEST_TYPE));
  EXPECT_TRUE(instruction.AddAttribute("value", REQUEST_VALUE));
  EXPECT_TRUE(instruction.AddAttribute("timeout", "0.1"));
  EXPECT_NO_THROW(instruction.Setup(proc));

  EXPECT_EQ(ui.m_log_entries.size(), 0);
  while (!IsFinishedStatus(instruction.GetStatus()))
  {
    EXPECT_NO_THROW(instruction.ExecuteSingle(ui, ws));
  }
  EXPECT_EQ(instruction.GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(RPCClientInstructionTest, MissingOutput)
{
  Procedure proc;
  Workspace ws;

  RPCClientInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("service", TEST_SERVICE_NAME));
  EXPECT_TRUE(instruction.AddAttribute("type", REQUEST_TYPE));
  EXPECT_TRUE(instruction.AddAttribute("value", REQUEST_VALUE));
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

TEST_F(RPCClientInstructionTest, ReadOnlyOutput)
{
  Procedure proc;
  Workspace ws;

  auto variable = std::make_unique<unit_test_helper::ReadOnlyVariable>(sup::dto::AnyValue{});
  EXPECT_NO_THROW(variable->Setup(ws));
  EXPECT_TRUE(ws.AddVariable("var", std::move(variable)));

  RPCClientInstruction instruction{};
  EXPECT_TRUE(instruction.AddAttribute("service", TEST_SERVICE_NAME));
  EXPECT_TRUE(instruction.AddAttribute("type", REQUEST_TYPE));
  EXPECT_TRUE(instruction.AddAttribute("value", REQUEST_VALUE));
  EXPECT_TRUE(instruction.AddAttribute("outputVar", "var"));
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
  EXPECT_NE(message.find("var"), std::string::npos);
}

TEST_F(RPCClientInstructionTest, Success) // Must be associated to a variable in the workspace
{
  auto proc = sup::oac_tree::ParseProcedureString(RPC_CLIENT_PROCEDURE);
  ASSERT_TRUE(static_cast<bool>(proc));
  EXPECT_NO_THROW(proc->Setup());

  sup::oac_tree::ExecutionStatus exec = sup::oac_tree::ExecutionStatus::FAILURE;
  do
  {
    proc->ExecuteSingle(ui);
    exec = proc->GetStatus();
  }
  while ((sup::oac_tree::ExecutionStatus::SUCCESS != exec) &&
         (sup::oac_tree::ExecutionStatus::FAILURE != exec));
  EXPECT_EQ(exec, ExecutionStatus::SUCCESS);

  sup::dto::AnyValue reply;
  proc->GetVariableValue("reply", reply);
  ASSERT_TRUE(sup::protocol::utils::CheckReplyFormat(reply));
  EXPECT_EQ(reply["result"].As<sup::dto::uint32>(), sup::protocol::Success.GetValue());
  EXPECT_TRUE(reply["reply"].As<sup::dto::boolean>());
}

TEST_F(RPCClientInstructionTest, VariableAttributes)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <RPCClient service="@service_name" requestVar="request" outputVar="reply" timeout="@mytimeout"/>
  <Workspace>
    <Local name="service_name" type='{"type":"string"}' value='"RPCClientInstructionTest::service"'/>
    <Local name="request"
           type='{"type":"sup::RPCRequest/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"query":{"type":"bool"}}]}'
           value='{"timestamp":0,"query":true}'/>
    <Local name="mytimeout" type='{"type":"float64"}' value='3.0'/>
    <Local name="reply"/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui));
}

TEST_F(RPCClientInstructionTest, VariableAttributesWrongType)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <RPCClient service="@service_name" requestVar="request" outputVar="reply" timeout="@mytimeout"/>
  <Workspace>
    <Local name="service_name" type='{"type":"string"}' value='"RPCClientInstructionTest::service"'/>
    <Local name="request"
           type='{"type":"sup::RPCRequest/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"query":{"type":"bool"}}]}'
           value='{"timestamp":0,"query":true}'/>
    <Local name="mytimeout" type='{"type":"string"}' value='"3.0"'/>
    <Local name="reply"/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}

TEST_F(RPCClientInstructionTest, VariableAttributesNotPresent)
{
  DefaultUserInterface ui;
  const std::string procedure_body{
R"RAW(
  <RPCClient service="@service_name" requestVar="request" outputVar="reply" timeout="@mytimeout"/>
  <Workspace>
    <Local name="service_name" type='{"type":"string"}' value='"RPCClientInstructionTest::service"'/>
    <Local name="request"
           type='{"type":"sup::RPCRequest/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"query":{"type":"bool"}}]}'
           value='{"timestamp":0,"query":true}'/>
    <Local name="reply"/>
  </Workspace>
)RAW"};

  const auto procedure_string = unit_test_helper::CreateProcedureString(procedure_body);
  auto proc = ParseProcedureString(procedure_string);
  EXPECT_TRUE(unit_test_helper::TryAndExecute(proc, ui, ExecutionStatus::FAILURE));
}

RPCClientInstructionTest::RPCClientInstructionTest() = default;
RPCClientInstructionTest::~RPCClientInstructionTest() = default;

void RPCClientInstructionTest::SetUpTestCase()
{
  auto config = sup::epics::GetDefaultRPCServerConfig(TEST_SERVICE_NAME);
  server = std::make_unique<sup::epics::PvAccessRPCServer>(config, m_handler);
}

void RPCClientInstructionTest::TearDownTestCase()
{
  server.reset();
}
