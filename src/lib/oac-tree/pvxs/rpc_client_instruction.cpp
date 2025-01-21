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
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include "rpc_client_instruction.h"
#include "pv_access_helper.h"

#include <sup/oac-tree/concrete_constraints.h>
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

const std::string SERVICE_ATTRIBUTE_NAME = "service";
const std::string REQUEST_ATTRIBUTE_NAME = "requestVar";
const std::string TYPE_ATTRIBUTE_NAME = "type";
const std::string VALUE_ATTRIBUTE_NAME = "value";
const std::string OUTPUT_ATTRIBUTE_NAME = "outputVar";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";

static bool _rpcclient_instruction_initialised_flag =
  RegisterGlobalInstruction<RPCClientInstruction>();

RPCClientInstruction::RPCClientInstruction()
  : Instruction(RPCClientInstruction::Type)
{
  AddAttributeDefinition(SERVICE_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
  AddAttributeDefinition(REQUEST_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName);
  AddAttributeDefinition(TYPE_ATTRIBUTE_NAME);
  AddAttributeDefinition(VALUE_ATTRIBUTE_NAME);
  AddAttributeDefinition(OUTPUT_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName);
  AddAttributeDefinition(TIMEOUT_ATTRIBUTE_NAME, sup::dto::Float64Type)
    .SetCategory(AttributeCategory::kBoth);
  AddConstraint(MakeConstraint<Xor>(
    MakeConstraint<Exists>(REQUEST_ATTRIBUTE_NAME),
    MakeConstraint<And>(MakeConstraint<Exists>(TYPE_ATTRIBUTE_NAME),
                        MakeConstraint<Exists>(VALUE_ATTRIBUTE_NAME))));
}

RPCClientInstruction::~RPCClientInstruction() = default;


ExecutionStatus RPCClientInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  auto request = GetRequest(ui, ws);
  if (sup::dto::IsEmptyValue(request))
  {
    return ExecutionStatus::FAILURE;
  }
  std::string service_name;
  if (!GetAttributeValueAs(SERVICE_ATTRIBUTE_NAME, ws, ui, service_name))
  {
    return ExecutionStatus::FAILURE;
  }
  auto client_config = sup::epics::GetDefaultRPCClientConfig(service_name);
  sup::dto::float64 timeout_sec = client_config.timeout;
  if (!GetAttributeValueAs(TIMEOUT_ATTRIBUTE_NAME, ws, ui, timeout_sec))
  {
    return ExecutionStatus::FAILURE;
  }
  if (timeout_sec < 0)
  {
    std::string error_message = InstructionSetupExceptionProlog(*this) +
      "timeout attribute is not positive: " + std::to_string(timeout_sec);
    LogError(ui, error_message);
    return ExecutionStatus::FAILURE;
  }
  client_config.timeout = timeout_sec;
  sup::epics::PvAccessRPCClient rpc_client(client_config);

  auto reply = rpc_client(request);
  if (HasAttribute(OUTPUT_ATTRIBUTE_NAME))
  {
    if (!SetValueFromAttributeName(*this, ws, ui, OUTPUT_ATTRIBUTE_NAME, reply))
    {
      return ExecutionStatus::FAILURE;
    }
  }
  return IsSuccessfulReply(reply) ? ExecutionStatus::SUCCESS
                                  : ExecutionStatus::FAILURE;
}

sup::dto::AnyValue RPCClientInstruction::GetRequest(UserInterface& ui, Workspace& ws)
{
  if (HasAttribute(REQUEST_ATTRIBUTE_NAME))
  {
    sup::dto::AnyValue request;
    if (!GetAttributeValue(REQUEST_ATTRIBUTE_NAME, ws, ui, request))
    {
      return {};
    }
    if (sup::dto::IsEmptyValue(request))
    {
      std::string warning_message = InstructionWarningProlog(*this) +
        "value from field [" + GetAttributeString(REQUEST_ATTRIBUTE_NAME) + "] is empty";
      LogWarning(ui, warning_message);
    }
    return request;
  }
  return ParseAnyValueAttributePair(*this, ws, ui, TYPE_ATTRIBUTE_NAME, VALUE_ATTRIBUTE_NAME);
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
