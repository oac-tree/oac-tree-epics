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

#include <sup/sequencer/concrete_constraints.h>
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
const std::string TYPE_ATTRIBUTE_NAME = "type";
const std::string VALUE_ATTRIBUTE_NAME = "value";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";

static bool _pv_access_write_instruction_initialised_flag =
  RegisterGlobalInstruction<PvAccessWriteInstruction>();

PvAccessWriteInstruction::PvAccessWriteInstruction()
  : Instruction(PvAccessWriteInstruction::Type)
  , m_timeout_sec{pv_access_helper::DEFAULT_TIMEOUT_SEC}
{
  AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
  AddAttributeDefinition(VARIABLE_NAME_ATTRIBUTE_NAME, sup::dto::StringType);
  AddAttributeDefinition(TYPE_ATTRIBUTE_NAME, sup::dto::StringType);
  AddAttributeDefinition(VALUE_ATTRIBUTE_NAME, sup::dto::StringType);
  AddAttributeDefinition(TIMEOUT_ATTRIBUTE_NAME, sup::dto::Float64Type);
  AddConstraint(MakeConstraint<Xor>(
    MakeConstraint<Exists>(VARIABLE_NAME_ATTRIBUTE_NAME),
    MakeConstraint<And>(MakeConstraint<Exists>(TYPE_ATTRIBUTE_NAME),
                        MakeConstraint<Exists>(VALUE_ATTRIBUTE_NAME))));
}

PvAccessWriteInstruction::~PvAccessWriteInstruction() = default;

void PvAccessWriteInstruction::SetupImpl(const Procedure&)
{
  if (HasAttribute(TIMEOUT_ATTRIBUTE_NAME))
  {
    auto timeout_val = GetAttributeValue<sup::dto::float64>(TIMEOUT_ATTRIBUTE_NAME);
    if (timeout_val < 0)
    {
      std::string error_message = InstructionSetupExceptionProlog(*this) +
        "attribute [" + TIMEOUT_ATTRIBUTE_NAME + "] with value [" +
        GetAttributeString(TIMEOUT_ATTRIBUTE_NAME) + "] is not positive";
      throw InstructionSetupException(error_message);
    }
    m_timeout_sec = timeout_val;
  }
}

void PvAccessWriteInstruction::ResetHook()
{
  m_timeout_sec = pv_access_helper::DEFAULT_TIMEOUT_SEC;
}

ExecutionStatus PvAccessWriteInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  auto value = pv_access_helper::PackIntoStructIfScalar(GetNewValue(ui, ws));
  if (sup::dto::IsEmptyValue(value))
  {
    return ExecutionStatus::FAILURE;
  }
  auto channel_name = GetAttributeValue<std::string>(CHANNEL_ATTRIBUTE_NAME);
  sup::epics::PvAccessClientPV pv(channel_name);
  if (!pv.WaitForConnected(m_timeout_sec))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "channel with name [" + channel_name + "] timed out";
    ui.LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!pv.SetValue(value))
  {
    auto json_value = sup::dto::ValuesToJSONString(value).substr(0, 1024);
    std::string warning_message = InstructionWarningProlog(*this) +
      "could not write value [" + json_value + "] to channel [" + channel_name + "]";
    ui.LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

sup::dto::AnyValue PvAccessWriteInstruction::GetNewValue(UserInterface& ui, Workspace& ws) const
{
  if (HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    sup::dto::AnyValue result;
    if (!GetValueFromAttributeName(*this, ws, ui, VARIABLE_NAME_ATTRIBUTE_NAME, result))
    {
      return {};
    }
    if (sup::dto::IsEmptyValue(result))
    {
      std::string warning_message =
        InstructionWarningProlog(*this) + "value from field [" +
        GetAttributeValue<std::string>(VARIABLE_NAME_ATTRIBUTE_NAME) + "] is empty";
      ui.LogWarning(warning_message);
    }
    return result;
  }
  auto type_str = GetAttributeValue<std::string>(TYPE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyTypeParser type_parser;
  if (!type_parser.ParseString(type_str, ws.GetTypeRegistry()))
  {
    std::string error_message = InstructionErrorProlog(*this) +
      "could not parse type [" + type_str + "] from attribute [" + TYPE_ATTRIBUTE_NAME + "]";
    ui.LogError(error_message);
    return {};
  }
  sup::dto::AnyType anytype = type_parser.MoveAnyType();
  auto val_str = GetAttributeValue<std::string>(VALUE_ATTRIBUTE_NAME);
  sup::dto::JSONAnyValueParser val_parser;
  if (!val_parser.TypedParseString(anytype, val_str))
  {
    std::string error_message = InstructionErrorProlog(*this) +
      "could not parse value [" + val_str + "] from attribute [" + VALUE_ATTRIBUTE_NAME +
      "] to type [" + type_str + "]";
    ui.LogError(error_message);
    return {};
  }
  return val_parser.MoveAnyValue();
}

} // namespace sequencer

} // namespace sup
