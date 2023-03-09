/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : Instruction node implementation
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

#include "rpc_client_instruction.h"
#include "pv_access_helper.h"

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/user_interface.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>
#include <sup/dto/json_value_parser.h>
#include <sup/epics/pv_access_rpc_client.h>
#include <sup/protocol/protocol_rpc.h>

#include <algorithm>

namespace
{
bool IsSuccessfulReply(sup::dto::AnyValue reply);
}  // unnamed namespace

namespace sup {

namespace sequencer {

const std::string RPCClientInstruction::Type = "RPCClient";

const std::string SERVICE_ATTRIBUTE_NAME = "service";
const std::string REQUEST_ATTRIBUTE_NAME = "requestVar";
const std::string TYPE_ATTRIBUTE_NAME = "type";
const std::string VALUE_ATTRIBUTE_NAME = "value";
const std::string OUTPUT_ATTRIBUTE_NAME = "output";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";

static bool _rpcclient_instruction_initialised_flag =
  RegisterGlobalInstruction<RPCClientInstruction>();

RPCClientInstruction::RPCClientInstruction()
  : Instruction(RPCClientInstruction::Type)
  , m_timeout{-1.0}
{}

RPCClientInstruction::~RPCClientInstruction() = default;

void RPCClientInstruction::SetupImpl(const Procedure&)
{
  CheckMandatoryNonEmptyAttribute(*this, SERVICE_ATTRIBUTE_NAME);
  if (!HasAttribute(REQUEST_ATTRIBUTE_NAME))
  {
    CheckMandatoryAttribute(*this, TYPE_ATTRIBUTE_NAME);
    CheckMandatoryAttribute(*this, VALUE_ATTRIBUTE_NAME);
  }
  if (HasAttribute(TIMEOUT_ATTRIBUTE_NAME))
  {
    auto timeout_str = GetAttribute(TIMEOUT_ATTRIBUTE_NAME);
    auto timeout_val = pv_access_helper::ParseTimeoutString(timeout_str);
    if (timeout_val < 0)
    {
      std::string error_message = InstructionSetupExceptionProlog(*this) +
        "could not parse attribute [" + TIMEOUT_ATTRIBUTE_NAME + "] with value [" + timeout_str +
        "] to positive or zero floating point value";
      throw InstructionSetupException(error_message);
    }
    m_timeout = timeout_val;
  }
}

void RPCClientInstruction::ResetHook()
{
  m_timeout = -1.0;
}

ExecutionStatus RPCClientInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  auto request = GetRequest(ui, ws);
  if (sup::dto::IsEmptyValue(request))
  {
    return ExecutionStatus::FAILURE;
  }
  auto service_name = GetAttribute(SERVICE_ATTRIBUTE_NAME);
  auto client_config = sup::epics::GetDefaultRPCClientConfig(GetAttribute(SERVICE_ATTRIBUTE_NAME));
  if (m_timeout >= 0.0)
  {
    client_config.timeout = m_timeout;
  }
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
    if (!GetValueFromAttributeName(*this, ws, ui, REQUEST_ATTRIBUTE_NAME, request))
    {
      return {};
    }
    if (sup::dto::IsEmptyValue(request))
    {
      std::string warning_message = InstructionWarningProlog(*this) +
        "value from field [" + GetAttribute(REQUEST_ATTRIBUTE_NAME) + "] is empty";
      ui.LogWarning(warning_message);
    }
    return request;
  }
  auto type_str = GetAttribute(TYPE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyTypeParser type_parser;
  if (!type_parser.ParseString(type_str, ws.GetTypeRegistry()))
  {
    std::string error_message = InstructionErrorProlog(*this) +
      "could not parse type [" + type_str + "] from attribute [" + TYPE_ATTRIBUTE_NAME + "]";
    ui.LogError(error_message);
    return {};
  }
  auto anytype = type_parser.MoveAnyType();
  auto val_str = GetAttribute(VALUE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyValueParser value_parser;
  if (!value_parser.TypedParseString(anytype, val_str))
  {
    std::string error_message = InstructionErrorProlog(*this) +
      "could not parse value [" + val_str + "] from attribute [" + VALUE_ATTRIBUTE_NAME +
      "] to type [" + type_str + "]";
    ui.LogError(error_message);
    return {};
  }
  return value_parser.MoveAnyValue();
}

} // namespace sequencer

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
