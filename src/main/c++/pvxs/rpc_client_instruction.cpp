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

#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/json_type_parser.h>
#include <sup/dto/json_value_parser.h>
#include <sup/epics/pv_access_rpc_client.h>
#include <sup/rpc/protocol_rpc.h>

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
{}

RPCClientInstruction::~RPCClientInstruction() = default;

bool RPCClientInstruction::SetupImpl(const Procedure& proc)
{
  if (!HasAttribute(SERVICE_ATTRIBUTE_NAME))
  {
    return false;
  }
  if (!HasAttribute(REQUEST_ATTRIBUTE_NAME) &&
      !(HasAttribute(TYPE_ATTRIBUTE_NAME) && HasAttribute(VALUE_ATTRIBUTE_NAME)))
  {
    return false;
  }
  if (HasAttribute(REQUEST_ATTRIBUTE_NAME))
  {
    auto var_names = proc.VariableNames();
    if (std::find(var_names.begin(), var_names.end(), GetAttribute(REQUEST_ATTRIBUTE_NAME))
        == var_names.end())
    {
      return false;
    }
  }
  if (HasAttribute(OUTPUT_ATTRIBUTE_NAME))
  {
    auto var_names = proc.VariableNames();
    if (std::find(var_names.begin(), var_names.end(), GetAttribute(OUTPUT_ATTRIBUTE_NAME))
        == var_names.end())
    {
      return false;
    }
  }
  return true;
}

ExecutionStatus RPCClientInstruction::ExecuteSingleImpl(UserInterface* ui, Workspace* ws)
{
  (void)ui;

  auto request = GetRequest(ws);
  auto timeout_str = HasAttribute(TIMEOUT_ATTRIBUTE_NAME) ? GetAttribute(TIMEOUT_ATTRIBUTE_NAME)
                                                          : "-1.0";
  auto timeout = pv_access_helper::ParseTimeoutString(timeout_str);
  auto client_config = sup::epics::GetDefaultRPCClientConfig(GetAttribute(SERVICE_ATTRIBUTE_NAME));
  if (timeout >= 0.0)
  {
    client_config.timeout = timeout;
  }
  sup::epics::PvAccessRPCClient rpc_client(client_config);

  auto reply = rpc_client(request);
  if (HasAttribute(OUTPUT_ATTRIBUTE_NAME))
  {
    if (!ws->SetValue(GetAttribute(OUTPUT_ATTRIBUTE_NAME), reply))
    {
      return ExecutionStatus::FAILURE;
    }
  }
  return IsSuccessfulReply(reply) ? ExecutionStatus::SUCCESS
                                  : ExecutionStatus::FAILURE;
}

sup::dto::AnyValue RPCClientInstruction::GetRequest(Workspace* ws)
{
  if (HasAttribute(REQUEST_ATTRIBUTE_NAME))
  {
    sup::dto::AnyValue request;
    if (!ws->GetValue(GetAttribute(REQUEST_ATTRIBUTE_NAME), request))
    {
      return {};
    }
    return request;
  }
  sup::dto::JSONAnyTypeParser type_parser;
  if (!type_parser.ParseString(GetAttribute(TYPE_ATTRIBUTE_NAME),
                               ws->GetTypeRegistry()))
  {
    return {};
  }
  auto anytype = type_parser.MoveAnyType();
  sup::dto::JSONAnyValueParser value_parser;
  if (!value_parser.TypedParseString(anytype, GetAttribute(VALUE_ATTRIBUTE_NAME)))
  {
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
  if (!sup::rpc::utils::CheckReplyFormat(reply))
  {
    return false;
  }
  return reply[sup::rpc::constants::REPLY_RESULT].As<sup::dto::uint32>() ==
         sup::rpc::Success.GetValue();
}
}  // unnamed namespace
