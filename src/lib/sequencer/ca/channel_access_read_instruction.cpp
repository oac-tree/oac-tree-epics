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

#include "channel_access_read_instruction.h"
#include "channel_access_helper.h"

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/user_interface.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/epics/channel_access_pv.h>

#include <algorithm>
#include <cmath>

namespace sup {

namespace sequencer {

const std::string ChannelAccessReadInstruction::Type = "ChannelAccessRead";

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string VARIABLE_NAME_ATTRIBUTE_NAME = "varName";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";

static bool _ca_read_instruction_initialised_flag =
  RegisterGlobalInstruction<ChannelAccessReadInstruction>();

ChannelAccessReadInstruction::ChannelAccessReadInstruction()
  : Instruction(ChannelAccessReadInstruction::Type)
  , m_timeout_sec{channel_access_helper::DEFAULT_TIMEOUT_SEC}
{}

ChannelAccessReadInstruction::~ChannelAccessReadInstruction() = default;

void ChannelAccessReadInstruction::SetupImpl(const Procedure&)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    std::string error_message = InstructionSetupExceptionProlog() +
      "missing mandatory attribute [" + CHANNEL_ATTRIBUTE_NAME + "]";
    throw InstructionSetupException(error_message);
  }
  if (!HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    std::string error_message = InstructionSetupExceptionProlog() +
      "missing mandatory attribute [" + VARIABLE_NAME_ATTRIBUTE_NAME + "]";
    throw InstructionSetupException(error_message);
  }
  if (HasAttribute(TIMEOUT_ATTRIBUTE_NAME))
  {
    auto timeout_str = GetAttribute(TIMEOUT_ATTRIBUTE_NAME);
    auto timeout_val = channel_access_helper::ParseTimeoutString(timeout_str);
    if (timeout_val < 0)
    {
      std::string error_message = InstructionSetupExceptionProlog() +
        "could not parse attribute [" + TIMEOUT_ATTRIBUTE_NAME + "] with value [" + timeout_str +
        "] to positive or zero floating point value";
      throw InstructionSetupException(error_message);
    }
    m_timeout_sec = timeout_val;
  }
}

void ChannelAccessReadInstruction::ResetHook()
{
  m_timeout_sec = channel_access_helper::DEFAULT_TIMEOUT_SEC;
}

ExecutionStatus ChannelAccessReadInstruction::ExecuteSingleImpl(UserInterface* ui, Workspace* ws)
{
  auto var_field_name = GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME);
  auto var_var_name = SplitFieldName(var_field_name).first;
  if (!ws->HasVariable(var_var_name))
  {
    std::string error_message = InstructionErrorLogProlog() +
      "workspace does not contain input variable with name [" + var_var_name + "]";
    ui->LogError(error_message);
    return ExecutionStatus::FAILURE;
  }
  sup::dto::AnyValue value;
  if (!ws->GetValue(var_field_name, value))
  {
    std::string warning_message = InstructionWarningLogProlog() +
      "could not read variable field with name [" + var_field_name + "] from workspace";
    ui->LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto channel_name = GetAttribute(CHANNEL_ATTRIBUTE_NAME);
  if (channel_name.empty())
  {
    std::string error_message = InstructionErrorLogProlog() +
      "channel name from attribute [" + CHANNEL_ATTRIBUTE_NAME + "] is empty";
    ui->LogError(error_message);
    return ExecutionStatus::FAILURE;
  }
  auto channel_type = channel_access_helper::ChannelType(value.GetType());
  if (sup::dto::IsEmptyType(channel_type))
  {
    std::string warning_message = InstructionWarningLogProlog() +
      "type of variable field with name [" + var_field_name + "] is Empty";
    ui->LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  sup::epics::ChannelAccessPV pv(channel_name, channel_type);
  if (!pv.WaitForValidValue(m_timeout_sec))
  {
    std::string warning_message = InstructionWarningLogProlog() +
      "channel with name [" + channel_name + "] timed out";
    ui->LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto ext_val = pv.GetExtendedValue();
  auto var_val = channel_access_helper::ConvertToTypedAnyValue(ext_val, value.GetType());
  if (sup::dto::IsEmptyValue(var_val))
  {
    std::string warning_message = InstructionWarningLogProlog() +
      "could not convert value from channel [" + channel_name +
      "] to type of variable field with name [" + var_field_name + "]";
    ui->LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!ws->SetValue(GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME), var_val))
  {
    std::string warning_message = InstructionWarningLogProlog() +
      "could not set value from channel [" + channel_name +
      "] to workspace variable field with name [" + var_field_name + "]";
    ui->LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

} // namespace sequencer

} // namespace sup
