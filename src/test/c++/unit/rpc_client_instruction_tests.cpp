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

#include "null_user_interface.h"
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

using namespace sup::sequencer;

class RPCTestHandler : public sup::dto::AnyFunctor
{
public:
  RPCTestHandler() = default;
  ~RPCTestHandler() = default;

  sup::dto::AnyValue operator()(const sup::dto::AnyValue& input) override
  {
    return sup::rpc::utils::CreateRPCReply(sup::rpc::Success, "", input);
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
  sup::sequencer::Procedure proc;
  ASSERT_TRUE(instruction->Setup(proc));
  Workspace ws;
  instruction->ExecuteSingle(&ui, &ws);
  EXPECT_EQ(instruction->GetStatus(), ExecutionStatus::FAILURE);
}

// TEST_F(RPCClientTest, Execute_success) // Must be associated to a variable in the workspace
// {

//   sup::sequencer::gtest::NullUserInterface ui;
//   auto proc = sup::sequencer::ParseProcedureString(
//     "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
//     "<Procedure xmlns=\"http://codac.iter.org/sup/sequencer\" version=\"1.0\"\n"
//     "           name=\"Trivial procedure for testing purposes\"\n"
//     "           xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
//     "           xs:schemaLocation=\"http://codac.iter.org/sup/sequencer sequencer.xsd\">\n"
//     "    <Sequence>\n"
//     "        <RPCClientInstruction name=\"rpc-client\"\n"
//     "            service=\"sequencer::test::rpc\"\n"
//     "            request=\"request\" reply=\"reply\"/>\n"
//     "        <LogTrace input=\"request\"/>\n"
//     "        <LogTrace input=\"reply\"/>\n"
//     "    </Sequence>\n"
//     "    <Workspace>\n"
//     "        <Local name=\"request\" type='{\"type\":\"Request_t\",\"attributes\":[{\"value\":{\"type\":\"uint32\"}},{\"status\":{\"type\":\"bool\"}}]}' value='{\"value\":1234u,\"status\":true}'/>\n"
//     "        <FileVariable name=\"reply\" file=\"/tmp/file-variable-struct.dat\"/>\n"
//     "    </Workspace>\n"
//     "</Procedure>");

//   bool status = static_cast<bool>(proc);

//   if (status)
//     { // Setup procedure
//       status = proc->Setup();
//     }

//   if (status)
//     {
//       status = Initialise();
//     }

//   if (status)
//     {
//       sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

//       do
//         {
//           (void)ccs::HelperTools::SleepFor(100000000ul); // Let system breathe
//           proc->ExecuteSingle(&ui);
//           exec = proc->GetStatus();
//         }
//       while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
//              (sup::sequencer::ExecutionStatus::FAILURE != exec));

//       status = (sup::sequencer::ExecutionStatus::SUCCESS == exec);
//     }

//   // Test variable

//   if (status)
//     {
//       ccs::types::AnyValue value;
//       status = ::ccs::HelperTools::ReadFromFile(&value, "/tmp/file-variable-struct.dat");

//       if (status)
// 	{
// 	  status = (::ccs::HelperTools::HasAttribute(&value, "value") && (1234u == ::ccs::HelperTools::GetAttributeValue<ccs::types::uint32>(&value, "value")));
// 	}
//     }

//   // Remove temp. files
//   (void)::ccs::HelperTools::ExecuteSystemCall("/usr/bin/rm -rf /tmp/file-variable-struct.dat");

//   (void)Terminate();

//   ASSERT_EQ(true, status);

// }

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
