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

#include <sup/sequencer/instruction.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/workspace.h>

#include <sup/epics/pv_access_rpc_client.h>

#include <algorithm>

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

void RPCClientInstruction::ResetHook()
{}

ExecutionStatus RPCClientInstruction::ExecuteSingleImpl(UserInterface* ui, Workspace* ws)
{
  (void)ui;

  auto request = GetRequest(ws);

  sup::epics::PvAccessRPCClient rpc_client(
    sup::epics::GetDefaultRPCClientConfig(GetAttribute(SERVICE_ATTRIBUTE_NAME)));

  auto reply = rpc_client(request);
  if (HasAttribute(OUTPUT_ATTRIBUTE_NAME))
  {
    if (!ws->SetValue(GetAttribute(OUTPUT_ATTRIBUTE_NAME), reply))
    {
      return ExecutionStatus::FAILURE;
    }
  }
  return ExecutionStatus::SUCCESS;
}

sup::dto::AnyValue RPCClientInstruction::GetRequest(Workspace* ws)
{
  (void)ws;
  return {};
}

} // namespace sequencer

} // namespace sup
