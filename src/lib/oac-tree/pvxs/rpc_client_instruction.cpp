/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : Instruction node implementation
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

#include "rpc_client_instruction.h"
#include "pv_access_helper.h"

#include <sup/oac-tree/concrete_constraints.h>
#include <sup/oac-tree/constants.h>
#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/instruction.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/procedure.h>
#include <sup/oac-tree/user_interface.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/epics/pv_access_rpc_client.h>
#include <sup/protocol/protocol_rpc.h>

#include <algorithm>

namespace
{
bool IsSuccessfulReply(sup::dto::AnyValue reply);
}  // unnamed namespace

namespace sup {

namespace oac_tree {

const std::string RPCClientInstruction::Type = "RPCClient";

static bool _rpcclient_instruction_initialised_flag =
  RegisterGlobalInstruction<RPCClientInstruction>();

RPCClientInstruction::RPCClientInstruction()
  : Instruction(RPCClientInstruction::Type)
  , m_future{}
{
  AddAttributeDefinition(pv_access_helper::SERVICE_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
  AddAttributeDefinition(pv_access_helper::REQUEST_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName);
  AddAttributeDefinition(Constants::TYPE_ATTRIBUTE_NAME);
  AddAttributeDefinition(Constants::VALUE_ATTRIBUTE_NAME);
  AddAttributeDefinition(Constants::OUTPUT_VARIABLE_NAME_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName);
  AddAttributeDefinition(Constants::TIMEOUT_SEC_ATTRIBUTE_NAME, sup::dto::Float64Type)
    .SetCategory(AttributeCategory::kBoth);
  AddConstraint(MakeConstraint<Xor>(
    MakeConstraint<Exists>(pv_access_helper::REQUEST_ATTRIBUTE_NAME),
    MakeConstraint<And>(MakeConstraint<Exists>(Constants::TYPE_ATTRIBUTE_NAME),
                        MakeConstraint<Exists>(Constants::VALUE_ATTRIBUTE_NAME))));
}

RPCClientInstruction::~RPCClientInstruction() = default;

bool RPCClientInstruction::InitHook(UserInterface& ui, Workspace& ws)
{
  auto request = GetRequest(ui, ws);
  if (sup::dto::IsEmptyValue(request))
  {
    return false;
  }
  std::string service_name;
  if (!GetAttributeValueAs(pv_access_helper::SERVICE_ATTRIBUTE_NAME, ws, ui, service_name))
  {
    return false;
  }
  auto client_config = sup::epics::GetDefaultRPCClientConfig(service_name);
  sup::dto::float64 timeout_sec = client_config.timeout;
  if (!GetAttributeValueAs(Constants::TIMEOUT_SEC_ATTRIBUTE_NAME, ws, ui, timeout_sec))
  {
    return false;
  }
  if (timeout_sec < 0)
  {
    std::string error_message = InstructionSetupExceptionProlog(*this) +
      "timeout attribute is not positive: " + std::to_string(timeout_sec);
    LogError(ui, error_message);
    return false;
  }
  client_config.timeout = timeout_sec;
  auto task = [client_config, request]() {
    sup::epics::PvAccessRPCClient rpc_client(client_config);
    return rpc_client(request);
  };
  m_future = std::async(std::launch::async, task);
  return true;
}

ExecutionStatus RPCClientInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  if (IsHaltRequested())
  {
    return ExecutionStatus::FAILURE;
  }
  if (m_future.wait_for(std::chrono::nanoseconds(0)) != std::future_status::ready)
  {
    return ExecutionStatus::RUNNING;
  }
  auto reply = m_future.get();
  if (HasAttribute(Constants::OUTPUT_VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    if (!SetValueFromAttributeName(*this, ws, ui, Constants::OUTPUT_VARIABLE_NAME_ATTRIBUTE_NAME,
                                   reply))
    {
      return ExecutionStatus::FAILURE;
    }
  }
  return IsSuccessfulReply(reply) ? ExecutionStatus::SUCCESS
                                  : ExecutionStatus::FAILURE;
}

void RPCClientInstruction::ResetHook(UserInterface& ui)
{
  (void)ui;
  Halt();
}

void RPCClientInstruction::HaltImpl()
{
  m_future = {};
}

sup::dto::AnyValue RPCClientInstruction::GetRequest(UserInterface& ui, Workspace& ws)
{
  if (HasAttribute(pv_access_helper::REQUEST_ATTRIBUTE_NAME))
  {
    sup::dto::AnyValue request;
    if (!GetAttributeValue(pv_access_helper::REQUEST_ATTRIBUTE_NAME, ws, ui, request))
    {
      return {};
    }
    if (sup::dto::IsEmptyValue(request))
    {
      std::string warning_message = InstructionWarningProlog(*this) + "value from field [" +
        GetAttributeString(pv_access_helper::REQUEST_ATTRIBUTE_NAME) + "] is empty";
      LogWarning(ui, warning_message);
    }
    return request;
  }
  return ParseAnyValueAttributePair(*this, ws, ui, Constants::TYPE_ATTRIBUTE_NAME,
                                    Constants::VALUE_ATTRIBUTE_NAME);
}

} // namespace oac_tree

} // namespace sup

namespace
{
bool IsSuccessfulReply(sup::dto::AnyValue reply)
{
  if (!sup::protocol::utils::CheckReplyFormat(reply))
  {
    return false;
  }
  return reply[sup::protocol::constants::REPLY_RESULT].As<sup::dto::uint32>() ==
         sup::protocol::Success.GetValue();
}
}  // unnamed namespace
