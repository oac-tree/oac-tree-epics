/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : Instruction node implementation
*
* Author        : Bertrand Bauvir (IO)
*
* Copyright (c) : 2010-2020 ITER Organization,
*                                 CS 90 046
*                                 13067 St. Paul-lez-Durance Cedex
*                                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

#include "channel_access_instruction.h"
#include "channel_access_helper.h"

#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/json_type_parser.h>
#include <sup/dto/json_value_parser.h>
#include <sup/epics/channel_access_pv.h>

#include <algorithm>
#include <cmath>

namespace sup {

namespace sequencer {

const std::string ChannelAccessReadInstruction::Type = "ChannelAccessRead";
const std::string ChannelAccessWriteInstruction::Type = "ChannelAccessWrite";

const long DEFAULT_TIMEOUT_SEC = 2.0;

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string VARIABLE_NAME_ATTRIBUTE_NAME = "varName";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";
const std::string TYPE_ATTRIBUTE_NAME = "type";
const std::string VALUE_ATTRIBUTE_NAME = "value";

static bool _caclient_initialised_flag =
  (RegisterGlobalInstruction<ChannelAccessReadInstruction>() &&
   RegisterGlobalInstruction<ChannelAccessWriteInstruction>());

ChannelAccessReadInstruction::ChannelAccessReadInstruction()
  : Instruction(ChannelAccessReadInstruction::Type)
  , m_timeout_sec{DEFAULT_TIMEOUT_SEC}
{}

ChannelAccessReadInstruction::~ChannelAccessReadInstruction() = default;

bool ChannelAccessReadInstruction::SetupImpl(const Procedure& proc)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME) || !HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    return false;
  }
  auto var_names = proc.VariableNames();
  if (std::find(var_names.begin(), var_names.end(), GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
      == var_names.end())
  {
    return false;
  }
  if (HasAttribute(TIMEOUT_ATTRIBUTE_NAME))
  {
    sup::dto::JSONAnyValueParser parser;
    if (!parser.TypedParseString(sup::dto::Float64Type, GetAttribute(TIMEOUT_ATTRIBUTE_NAME)))
    {
      return false;
    }
    auto timeout_val = parser.MoveAnyValue().As<sup::dto::float64>();
    if (timeout_val < 0)
    {
      return false;
    }
    m_timeout_sec = timeout_val;
  }
  return true;
}

void ChannelAccessReadInstruction::ResetHook()
{
  m_timeout_sec = DEFAULT_TIMEOUT_SEC;
}

ExecutionStatus ChannelAccessReadInstruction::ExecuteSingleImpl(UserInterface*, Workspace* ws)
{
  sup::dto::AnyValue value;
  if (!ws->GetValue(GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME), value))
  {
    return ExecutionStatus::FAILURE;
  }
  sup::epics::ChannelAccessPV pv(GetAttribute(CHANNEL_ATTRIBUTE_NAME),
                                 channel_access_helper::ChannelType(value.GetType()));
  if (!pv.WaitForConnected(m_timeout_sec))
  {
    return ExecutionStatus::FAILURE;
  }
  auto ext_val = pv.GetExtendedValue();
  auto var_val = channel_access_helper::ConvertToTypedAnyValue(ext_val, value.GetType());
  if (sup::dto::IsEmptyValue(var_val))
  {
    return ExecutionStatus::FAILURE;
  }
  if (!ws->SetValue(GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME), var_val))
  {
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

ChannelAccessWriteInstruction::ChannelAccessWriteInstruction()
  : Instruction(ChannelAccessWriteInstruction::Type)
  , m_timeout_sec{DEFAULT_TIMEOUT_SEC}
{}

ChannelAccessWriteInstruction::~ChannelAccessWriteInstruction() = default;

bool ChannelAccessWriteInstruction::SetupImpl(const Procedure& proc)
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
  if (HasAttribute(TIMEOUT_ATTRIBUTE_NAME))
  {
    sup::dto::JSONAnyValueParser parser;
    if (!parser.TypedParseString(sup::dto::Float64Type, GetAttribute(TIMEOUT_ATTRIBUTE_NAME)))
    {
      return false;
    }
    auto timeout_val = parser.MoveAnyValue().As<sup::dto::float64>();
    if (timeout_val < 0)
    {
      return false;
    }
    m_timeout_sec = timeout_val;
  }
  return true;
}

void ChannelAccessWriteInstruction::ResetHook()
{
  m_timeout_sec = DEFAULT_TIMEOUT_SEC;
}

ExecutionStatus ChannelAccessWriteInstruction::ExecuteSingleImpl (UserInterface* ui, Workspace* ws)
{
  auto value = channel_access_helper::ExtractChannelValue(GetNewValue(ws));
  sup::epics::ChannelAccessPV pv(GetAttribute(CHANNEL_ATTRIBUTE_NAME), value.GetType());
  if (!pv.WaitForConnected(m_timeout_sec))
  {
    return ExecutionStatus::FAILURE;
  }
  if (!pv.SetValue(value))
  {
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

sup::dto::AnyValue ChannelAccessWriteInstruction::GetNewValue(Workspace* ws) const
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
  if (!type_parser.ParseString(type_str))
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
