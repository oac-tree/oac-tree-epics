/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP Sequencer
 *
 * Description   : Unit test code
 *
 * Author        : Walter Van Herck (IO)
 *
 * Copyright (c) : 2010-2022 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include "test_user_interface.h"
#include "unit_test_helper.h"

#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/sequence_parser.h>
#include <sup/sequencer/workspace.h>

#include <sup/epics/pv_access_rpc_server.h>

#include <sup/rpc/protocol_rpc.h>

#include <gtest/gtest.h>

#include <memory>

static const std::string TEST_SERVICE_NAME = "RPCClientTest::service";

static const std::string RPC_CLIENT_PROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <Sequence>
        <RPCClient name="rpc-client"
                   service="RPCClientTest::service"
                   requestVar="request"
                   output="reply"/>
    </Sequence>
    <Workspace>
        <Local name="request"
               type='{"type":"sup::RPCRequest/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"query":{"type":"bool"}}]}'
               value='{"timestamp":0,"query":true}'/>
        <Local name="reply"/>
    </Workspace>
</Procedure>)RAW";


using namespace sup::sequencer;

class RPCTestHandler : public sup::dto::AnyFunctor
{
public:
  RPCTestHandler() = default;
  ~RPCTestHandler() = default;

  sup::dto::AnyValue operator()(const sup::dto::AnyValue& input) override
  {
    return sup::rpc::utils::CreateRPCReply(sup::rpc::Success, "", input["query"]);
  }
};

class RPCClientTest : public ::testing::Test
{
protected:
  RPCClientTest();
  ~RPCClientTest();

  static void SetUpTestCase();
  static void TearDownTestCase();

  static std::unique_ptr<sup::epics::PvAccessRPCServer> server;

  unit_test_helper::NullUserInterface ui;
};

std::unique_ptr<sup::epics::PvAccessRPCServer> RPCClientTest::server{};

TEST_F(RPCClientTest, Execute_missing)
{
  auto instruction = GlobalInstructionRegistry().Create("RPCClient");

  ASSERT_TRUE(static_cast<bool>(instruction));

  unit_test_helper::NullUserInterface ui;
  Workspace ws;
  instruction->ExecuteSingle(&ui, &ws);
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(RPCClientTest, Execute_undefined)
{
  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("RPCClient");
  ASSERT_TRUE(static_cast<bool>(instruction));
  EXPECT_TRUE(instruction->AddAttribute("service", "undefined"));
  EXPECT_TRUE(instruction->AddAttribute("type", "{\"type\": \"string\"}"));
  EXPECT_TRUE(instruction->AddAttribute("value", "\"undefined\""));
  EXPECT_TRUE(instruction->AddAttribute("timeout", "0.5"));
  sup::sequencer::Procedure proc;
  ASSERT_TRUE(instruction->Setup(proc));
  Workspace ws;
  instruction->ExecuteSingle(&ui, &ws);
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::FAILURE);
}

TEST_F(RPCClientTest, Execute_success) // Must be associated to a variable in the workspace
{
  auto proc = sup::sequencer::ParseProcedureString(RPC_CLIENT_PROCEDURE);
  ASSERT_TRUE(static_cast<bool>(proc));
  EXPECT_TRUE(proc->Setup());

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  }
  while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
         (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, ExecutionStatus::SUCCESS);

  sup::dto::AnyValue reply;
  proc->GetVariableValue("reply", reply);
  ASSERT_TRUE(sup::rpc::utils::CheckReplyFormat(reply));
  EXPECT_EQ(reply["result"].As<sup::dto::uint32>(), sup::rpc::Success.GetValue());
  EXPECT_TRUE(reply["reply"].As<sup::dto::boolean>());
}

RPCClientTest::RPCClientTest() = default;
RPCClientTest::~RPCClientTest() = default;

void RPCClientTest::SetUpTestCase()
{
  auto config = sup::epics::GetDefaultRPCServerConfig(TEST_SERVICE_NAME);
  std::unique_ptr<sup::dto::AnyFunctor> handler{new RPCTestHandler{}};
  server.reset(new sup::epics::PvAccessRPCServer(config, std::move(handler)));
}

void RPCClientTest::TearDownTestCase()
{
  server.reset();
}
