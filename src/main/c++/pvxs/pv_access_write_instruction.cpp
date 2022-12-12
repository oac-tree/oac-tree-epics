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

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/user_interface.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/anyvalue_helper.h>
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

void PvAccessWriteInstruction::SetupImpl(const Procedure&)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    std::string error_message = InstructionSetupExceptionProlog(GetName(), Type) +
      "missing mandatory attribute [" + CHANNEL_ATTRIBUTE_NAME + "]";
    throw InstructionSetupException(error_message);
  }
  if (!HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME) &&
     (!HasAttribute(TYPE_ATTRIBUTE_NAME) || !HasAttribute(VALUE_ATTRIBUTE_NAME)))
  {
    std::string error_message = InstructionSetupExceptionProlog(GetName(), Type) +
      "instruction requires either attribute [" + VARIABLE_NAME_ATTRIBUTE_NAME +
      "] or both attributes [" + TYPE_ATTRIBUTE_NAME + ", " + VALUE_ATTRIBUTE_NAME + "]";
    throw InstructionSetupException(error_message);
  }
}

ExecutionStatus PvAccessWriteInstruction::ExecuteSingleImpl(UserInterface* ui, Workspace* ws)
{
  auto value = pv_access_helper::PackIntoStructIfScalar(GetNewValue(ui, ws));
  if (sup::dto::IsEmptyValue(value))
  {
    std::string warning_message = InstructionWarningLogProlog(GetName(), Type) +
      "value to write is Empty";
    ui->LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto channel_name = GetAttribute(CHANNEL_ATTRIBUTE_NAME);
  if (channel_name.empty())
  {
    std::string error_message = InstructionErrorLogProlog(GetName(), Type) +
      "channel name from attribute [" + CHANNEL_ATTRIBUTE_NAME + "] is empty";
    ui->LogError(error_message);
    return ExecutionStatus::FAILURE;
  }
  sup::epics::PvAccessClientPV pv(channel_name);
  auto timeout_str = HasAttribute(TIMEOUT_ATTRIBUTE_NAME) ? GetAttribute(TIMEOUT_ATTRIBUTE_NAME)
                                                          : "-1.0";
  auto timeout = pv_access_helper::ParseTimeoutString(timeout_str);
  if (timeout >= 0.0 && !pv.WaitForConnected(timeout))
  {
    std::string warning_message = InstructionWarningLogProlog(GetName(), Type) +
      "channel with name [" + channel_name + "] timed out";
    ui->LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!pv.SetValue(value))
  {
    auto json_value = sup::dto::ValuesToJSONString(value).substr(0, 1024);
    std::string warning_message = InstructionWarningLogProlog(GetName(), Type) +
      "could not write value [" + json_value + "] to channel [" + channel_name + "]";
    ui->LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

sup::dto::AnyValue PvAccessWriteInstruction::GetNewValue(UserInterface* ui, Workspace* ws) const
{
  if (HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    auto var_field_name = GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME);
    auto var_var_name = SplitFieldName(var_field_name).first;
    if (!ws->HasVariable(var_var_name))
    {
      std::string error_message = InstructionErrorLogProlog(GetName(), Type) +
        "workspace does not contain input variable with name [" + var_var_name + "]";
      ui->LogError(error_message);
      return {};
    }
    sup::dto::AnyValue result;
    if (!ws->GetValue(var_field_name, result))
    {
      std::string error_message = InstructionErrorLogProlog(GetName(), Type) +
        "could not read variable field with name [" + var_field_name + "] from workspace";
      ui->LogError(error_message);
      return {};
    }
    return result;
  }
  auto type_str = GetAttribute(TYPE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyTypeParser type_parser;
  if (!type_parser.ParseString(type_str, ws->GetTypeRegistry()))
  {
    std::string error_message = InstructionErrorLogProlog(GetName(), Type) +
      "could not parse type [" + type_str + "] from attribute [" + TYPE_ATTRIBUTE_NAME + "]";
    ui->LogError(error_message);
    return {};
  }
  sup::dto::AnyType anytype = type_parser.MoveAnyType();
  auto val_str = GetAttribute(VALUE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyValueParser val_parser;
  if (!val_parser.TypedParseString(anytype, val_str))
  {
    std::string error_message = InstructionErrorLogProlog(GetName(), Type) +
      "could not parse value [" + val_str + "] from attribute [" + VALUE_ATTRIBUTE_NAME +
      "] to type [" + type_str + "]";
    ui->LogError(error_message);
    return {};
  }
  return val_parser.MoveAnyValue();
}

} // namespace sequencer

} // namespace sup
