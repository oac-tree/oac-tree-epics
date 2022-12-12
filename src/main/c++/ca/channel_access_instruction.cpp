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

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/log_severity.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/user_interface.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/anyvalue_helper.h>
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

void ChannelAccessReadInstruction::SetupImpl(const Procedure&)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    std::string error_message =
      "sup::sequencer::ChannelAccessReadInstruction::SetupImpl(): missing mandatory attribute [" +
       CHANNEL_ATTRIBUTE_NAME + "]";
    throw InstructionSetupException(error_message);
  }
  if (!HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    std::string error_message =
      "sup::sequencer::ChannelAccessReadInstruction::SetupImpl(): missing mandatory attribute [" +
       VARIABLE_NAME_ATTRIBUTE_NAME + "]";
    throw InstructionSetupException(error_message);
  }
  if (HasAttribute(TIMEOUT_ATTRIBUTE_NAME))
  {
    sup::dto::JSONAnyValueParser parser;
    auto timeout_str = GetAttribute(TIMEOUT_ATTRIBUTE_NAME);
    if (!parser.TypedParseString(sup::dto::Float64Type, timeout_str))
    {
      std::string error_message =
        "sup::sequencer::ChannelAccessReadInstruction::SetupImpl(): could not parse attribute [" +
         TIMEOUT_ATTRIBUTE_NAME + "] with value [" + timeout_str + "] to sup::dto::Float64Type";
      throw InstructionSetupException(error_message);
    }
    auto timeout_val = parser.MoveAnyValue().As<sup::dto::float64>();
    if (timeout_val < 0)
    {
      std::string error_message =
        "sup::sequencer::ChannelAccessReadInstruction::SetupImpl(): attribute [" +
         TIMEOUT_ATTRIBUTE_NAME + "] with value [" + timeout_str + "] should be positive or zero";
      throw InstructionSetupException(error_message);
    }
    m_timeout_sec = timeout_val;
  }
}

void ChannelAccessReadInstruction::ResetHook()
{
  m_timeout_sec = DEFAULT_TIMEOUT_SEC;
}

ExecutionStatus ChannelAccessReadInstruction::ExecuteSingleImpl(UserInterface* ui, Workspace* ws)
{
  auto var_field_name = GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME);
  auto var_var_name = SplitFieldName(var_field_name).first;
  if (!ws->HasVariable(var_var_name))
  {
    std::string error_message =
      "sup::sequencer::ChannelAccessReadInstruction::ExecuteSingleImpl(): workspace does not "
      "contain input variable with name [" + var_var_name + "]";
    ui->Log(log::SUP_SEQ_LOG_ERR, error_message);
    return ExecutionStatus::FAILURE;
  }
  sup::dto::AnyValue value;
  if (!ws->GetValue(var_field_name, value))
  {
    std::string warning_message =
      "sup::sequencer::ChannelAccessReadInstruction::ExecuteSingleImpl(): could not read variable "
      "field with name [" + var_field_name + "] from workspace";
    ui->Log(log::SUP_SEQ_LOG_WARNING, warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto channel_name = GetAttribute(CHANNEL_ATTRIBUTE_NAME);
  if (channel_name.empty())
  {
    std::string error_message =
      "sup::sequencer::ChannelAccessReadInstruction::ExecuteSingleImpl(): channel name from "
      "attribute [" + CHANNEL_ATTRIBUTE_NAME + "] is empty";
    ui->Log(log::SUP_SEQ_LOG_ERR, error_message);
    return ExecutionStatus::FAILURE;
  }
  auto channel_type = channel_access_helper::ChannelType(value.GetType());
  if (sup::dto::IsEmptyType(channel_type))
  {
    std::string warning_message =
      "sup::sequencer::ChannelAccessReadInstruction::ExecuteSingleImpl(): type of variable "
      "field with name [" + var_field_name + "] is Empty";
    ui->Log(log::SUP_SEQ_LOG_WARNING, warning_message);
    return ExecutionStatus::FAILURE;
  }
  sup::epics::ChannelAccessPV pv(channel_name, channel_type);
  if (!pv.WaitForValidValue(m_timeout_sec))
  {
    std::string warning_message =
      "sup::sequencer::ChannelAccessReadInstruction::ExecuteSingleImpl(): channel with name [" +
      channel_name + "] timed out";
    ui->Log(log::SUP_SEQ_LOG_WARNING, warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto ext_val = pv.GetExtendedValue();
  auto var_val = channel_access_helper::ConvertToTypedAnyValue(ext_val, value.GetType());
  if (sup::dto::IsEmptyValue(var_val))
  {
    std::string warning_message =
      "sup::sequencer::ChannelAccessReadInstruction::ExecuteSingleImpl(): could not convert "
      "value from channel [" + channel_name + "] to type of variable field with name [" +
      var_field_name + "]";
    ui->Log(log::SUP_SEQ_LOG_WARNING, warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!ws->SetValue(GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME), var_val))
  {
    std::string warning_message =
      "sup::sequencer::ChannelAccessReadInstruction::ExecuteSingleImpl(): could not set "
      "value from channel [" + channel_name + "] to workspace variable field with name [" +
      var_field_name + "]";
    ui->Log(log::SUP_SEQ_LOG_WARNING, warning_message);
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

ChannelAccessWriteInstruction::ChannelAccessWriteInstruction()
  : Instruction(ChannelAccessWriteInstruction::Type)
  , m_timeout_sec{DEFAULT_TIMEOUT_SEC}
{}

ChannelAccessWriteInstruction::~ChannelAccessWriteInstruction() = default;

void ChannelAccessWriteInstruction::SetupImpl(const Procedure&)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    std::string error_message =
      "sup::sequencer::ChannelAccessWriteInstruction::SetupImpl(): missing mandatory attribute [" +
       CHANNEL_ATTRIBUTE_NAME + "]";
    throw InstructionSetupException(error_message);
  }
  if (!HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME) &&
     (!HasAttribute(TYPE_ATTRIBUTE_NAME) || !HasAttribute(VALUE_ATTRIBUTE_NAME)))
  {
    std::string error_message =
      "sup::sequencer::ChannelAccessWriteInstruction::SetupImpl(): instruction requires either "
      "attribute [" + VARIABLE_NAME_ATTRIBUTE_NAME + "] or both attributes [" +
      TYPE_ATTRIBUTE_NAME + ", " + VALUE_ATTRIBUTE_NAME + "]";
    throw InstructionSetupException(error_message);
  }
  if (HasAttribute(TIMEOUT_ATTRIBUTE_NAME))
  {
    sup::dto::JSONAnyValueParser parser;
    auto timeout_str = GetAttribute(TIMEOUT_ATTRIBUTE_NAME);
    if (!parser.TypedParseString(sup::dto::Float64Type, timeout_str))
    {
      std::string error_message =
        "sup::sequencer::ChannelAccessWriteInstruction::SetupImpl(): could not parse attribute [" +
         TIMEOUT_ATTRIBUTE_NAME + "] with value [" + timeout_str + "] to sup::dto::Float64Type";
      throw InstructionSetupException(error_message);
    }
    auto timeout_val = parser.MoveAnyValue().As<sup::dto::float64>();
    if (timeout_val < 0)
    {
      std::string error_message =
        "sup::sequencer::ChannelAccessWriteInstruction::SetupImpl(): attribute [" +
         TIMEOUT_ATTRIBUTE_NAME + "] with value [" + timeout_str + "] should be positive or zero";
      throw InstructionSetupException(error_message);
    }
    m_timeout_sec = timeout_val;
  }
}

void ChannelAccessWriteInstruction::ResetHook()
{
  m_timeout_sec = DEFAULT_TIMEOUT_SEC;
}

ExecutionStatus ChannelAccessWriteInstruction::ExecuteSingleImpl(UserInterface* ui, Workspace* ws)
{
  (void)ui;
  auto value = channel_access_helper::ExtractChannelValue(GetNewValue(ui, ws));
  if (sup::dto::IsEmptyValue(value))
  {
    std::string warning_message =
      "sup::sequencer::ChannelAccessWriteInstruction::ExecuteSingleImpl(): value to write is Empty";
    ui->Log(log::SUP_SEQ_LOG_WARNING, warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto channel_name = GetAttribute(CHANNEL_ATTRIBUTE_NAME);
  if (channel_name.empty())
  {
    std::string error_message =
      "sup::sequencer::ChannelAccessWriteInstruction::ExecuteSingleImpl(): channel name from "
      "attribute [" + CHANNEL_ATTRIBUTE_NAME + "] is empty";
    ui->Log(log::SUP_SEQ_LOG_ERR, error_message);
    return ExecutionStatus::FAILURE;
  }
  auto channel_type = value.GetType();
  sup::epics::ChannelAccessPV pv(channel_name, channel_type);
  if (!pv.WaitForConnected(m_timeout_sec))
  {
    std::string warning_message =
      "sup::sequencer::ChannelAccessWriteInstruction::ExecuteSingleImpl(): channel with name [" +
      channel_name + "] timed out";
    ui->Log(log::SUP_SEQ_LOG_WARNING, warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!pv.SetValue(value))
  {
    auto json_value = sup::dto::ValuesToJSONString(value);
    std::string warning_message =
      "sup::sequencer::ChannelAccessWriteInstruction::ExecuteSingleImpl(): could not write value "
      "[" + json_value + "] to channel [" + channel_name + "]";
    ui->Log(log::SUP_SEQ_LOG_WARNING, warning_message);
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

sup::dto::AnyValue ChannelAccessWriteInstruction::GetNewValue(UserInterface* ui,
                                                              Workspace* ws) const
{
  if (HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    auto var_field_name = GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME);
    auto var_var_name = SplitFieldName(var_field_name).first;
    if (!ws->HasVariable(var_var_name))
    {
      std::string error_message =
        "sup::sequencer::ChannelAccessWriteInstruction::GetNewValue(): workspace does not "
        "contain input variable with name [" + var_var_name + "]";
      ui->Log(log::SUP_SEQ_LOG_ERR, error_message);
      return {};
    }
    sup::dto::AnyValue result;
    if (!ws->GetValue(var_field_name, result))
    {
      std::string error_message =
        "sup::sequencer::ChannelAccessWriteInstruction::GetNewValue(): could not read variable "
        "field with name [" + var_field_name + "] from workspace";
      ui->Log(log::SUP_SEQ_LOG_ERR, error_message);
      return {};
    }
    return result;
  }
  auto type_str = GetAttribute(TYPE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyTypeParser type_parser;
  auto registry = (ws == nullptr) ? nullptr : ws->GetTypeRegistry();
  if (!type_parser.ParseString(type_str, registry))
  {
    std::string error_message =
      "sup::sequencer::ChannelAccessWriteInstruction::GetNewValue(): could not parse type [" +
      type_str + "] from attribute [" + TYPE_ATTRIBUTE_NAME + "]";
    ui->Log(log::SUP_SEQ_LOG_ERR, error_message);
    return {};
  }
  sup::dto::AnyType anytype = type_parser.MoveAnyType();
  auto val_str = GetAttribute(VALUE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyValueParser val_parser;
  if (!val_parser.TypedParseString(anytype, val_str))
  {
    std::string error_message =
      "sup::sequencer::ChannelAccessWriteInstruction::GetNewValue(): could not parse value [" +
      val_str + "] from attribute [" + VALUE_ATTRIBUTE_NAME + "] to type [" + type_str + "]";
    ui->Log(log::SUP_SEQ_LOG_ERR, error_message);
    return {};
  }
  return val_parser.MoveAnyValue();
}

} // namespace sequencer

} // namespace sup
