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
*                                 CS 90 046
*                                 13067 St. Paul-lez-Durance Cedex
*                                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

#include "pv_access_write_instruction.h"

#include "pv_access_helper.h"

#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/json_type_parser.h>
#include <sup/dto/json_value_parser.h>
#include <sup/epics/pv_access_client_pv.h>

#include <algorithm>
#include <cmath>

namespace sup {

namespace sequencer {

const std::string PvAccessWriteInstruction::Type = "PvAccessWrite";

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string VARIABLE_NAME_ATTRIBUTE_NAME = "varName";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";
const std::string TYPE_ATTRIBUTE_NAME = "type";
const std::string VALUE_ATTRIBUTE_NAME = "value";

static bool _pv_access_write_instruction_initialised_flag =
  RegisterGlobalInstruction<PvAccessWriteInstruction>();

PvAccessWriteInstruction::PvAccessWriteInstruction()
  : Instruction(PvAccessWriteInstruction::Type)
{}

PvAccessWriteInstruction::~PvAccessWriteInstruction() = default;

bool PvAccessWriteInstruction::SetupImpl(const Procedure& proc)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    return false;
  }
  if (!HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME) &&
     (!HasAttribute(TYPE_ATTRIBUTE_NAME) || !HasAttribute(VALUE_ATTRIBUTE_NAME)))
  {
    return false;
  }
  if (HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    auto var_names = proc.VariableNames();
    if (std::find(var_names.begin(), var_names.end(), GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
        == var_names.end())
    {
      return false;
    }
  }
  return true;
}

ExecutionStatus PvAccessWriteInstruction::ExecuteSingleImpl(UserInterface* ui, Workspace* ws)
{
  (void)ui;
  auto value = pv_access_helper::PackIntoStructIfScalar(GetNewValue(ws));
  auto channel_name = GetAttribute(CHANNEL_ATTRIBUTE_NAME);
  sup::epics::PvAccessClientPV pv(channel_name);
  auto timeout_str = HasAttribute(TIMEOUT_ATTRIBUTE_NAME) ? GetAttribute(TIMEOUT_ATTRIBUTE_NAME)
                                                          : "-1.0";
  auto timeout = pv_access_helper::ParseTimeoutString(timeout_str);
  if (timeout >= 0.0 && !pv.WaitForConnected(timeout))
  {
    return ExecutionStatus::FAILURE;
  }
  if (!pv.SetValue(value))
  {
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

sup::dto::AnyValue PvAccessWriteInstruction::GetNewValue(Workspace* ws) const
{
  if (HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    sup::dto::AnyValue result;
    if (!ws->GetValue(GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME), result))
    {
      return {};
    }
    return result;
  }
  auto type_str = GetAttribute(TYPE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyTypeParser type_parser;
  auto registry = (ws == nullptr) ? nullptr : ws->GetTypeRegistry();
  if (!type_parser.ParseString(type_str, registry))
  {
    return {};
  }
  sup::dto::AnyType anytype = type_parser.MoveAnyType();
  auto val_str = GetAttribute(VALUE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyValueParser val_parser;
  if (!val_parser.TypedParseString(anytype, val_str))
  {
    return {};
  }
  return val_parser.MoveAnyValue();
}

} // namespace sequencer

} // namespace sup
