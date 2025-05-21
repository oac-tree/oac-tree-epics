/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) oac-tree component
*
* Description   : Instruction node implementation
*
* Author        : Bertrand Bauvir (IO)
*
* Copyright (c) : 2010-2025 ITER Organization,
*                                 CS 90 046
*                                 13067 St. Paul-lez-Durance Cedex
*                                 France
* SPDX-License-Identifier: MIT
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file LICENSE located in the top level directory
* of the distribution package.
******************************************************************************/

#include "channel_access_read_instruction.h"
#include "channel_access_helper.h"

#include <sup/oac-tree/constants.h>
#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/generic_utils.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/instruction_utils.h>
#include <sup/oac-tree/user_interface.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/epics/channel_access_pv.h>

#include <algorithm>
#include <cmath>

namespace sup {

namespace oac_tree {

const std::string ChannelAccessReadInstruction::Type = "ChannelAccessRead";

static bool _ca_read_instruction_initialised_flag =
  RegisterGlobalInstruction<ChannelAccessReadInstruction>();

ChannelAccessReadInstruction::ChannelAccessReadInstruction()
  : Instruction(ChannelAccessReadInstruction::Type)
  , m_channel_name{}
  , m_var_field_name{}
  , m_var_type{}
  , m_finish{}
  , m_pv{}
{
  AddAttributeDefinition(channel_access_helper::CHANNEL_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
  AddAttributeDefinition(Constants::OUTPUT_VARIABLE_NAME_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName).SetMandatory();
  AddAttributeDefinition(Constants::TIMEOUT_SEC_ATTRIBUTE_NAME, sup::dto::Float64Type)
    .SetCategory(AttributeCategory::kBoth);
}

ChannelAccessReadInstruction::~ChannelAccessReadInstruction() = default;

bool ChannelAccessReadInstruction::InitHook(UserInterface& ui, Workspace& ws)
{
  if (!GetAttributeValueAs(channel_access_helper::CHANNEL_ATTRIBUTE_NAME, ws, ui, m_channel_name))
  {
    return false;
  }
  sup::dto::AnyValue value;
  if (!GetAttributeValue(Constants::OUTPUT_VARIABLE_NAME_ATTRIBUTE_NAME, ws, ui, value))
  {
    return false;
  }
  m_var_field_name = GetAttributeString(Constants::OUTPUT_VARIABLE_NAME_ATTRIBUTE_NAME);
  m_var_type = value.GetType();
  auto channel_type = channel_access_helper::ChannelType(m_var_type);
  if (sup::dto::IsEmptyType(channel_type))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "type of variable field with name [" + m_var_field_name + "] is Empty";
    LogWarning(ui, warning_message);
    return false;
  }
  sup::dto::int64 timeout_ns = channel_access_helper::DEFAULT_TIMEOUT_NS;
  if (!instruction_utils::GetVariableTimeoutAttribute(
            *this, ui, ws, Constants::TIMEOUT_SEC_ATTRIBUTE_NAME, timeout_ns))
  {
    return false;
  }
  m_finish = utils::GetNanosecsSinceEpoch() + timeout_ns;
  m_pv = std::make_unique<sup::epics::ChannelAccessPV>(m_channel_name, channel_type);
  return true;
}

ExecutionStatus ChannelAccessReadInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  if (IsHaltRequested())
  {
    return ExecutionStatus::FAILURE;
  }
  auto now = utils::GetNanosecsSinceEpoch();
  auto ext_val = m_pv->GetExtendedValue();
  if (!ext_val.connected || sup::dto::IsEmptyValue(ext_val.value))
  {
    if (m_finish > now)
    {
      return ExecutionStatus::RUNNING;
    }
    std::string warning_message = InstructionWarningProlog(*this) +
      "channel with name [" + m_channel_name + "] timed out";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto var_val = channel_access_helper::ConvertToTypedAnyValue(ext_val, m_var_type);
  if (sup::dto::IsEmptyValue(var_val))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "could not convert value from channel [" + m_channel_name +
      "] to type of variable field with name [" + m_var_field_name + "]";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!SetValueFromAttributeName(*this, ws, ui, Constants::OUTPUT_VARIABLE_NAME_ATTRIBUTE_NAME, ext_val.value))
  {
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

void ChannelAccessReadInstruction::ResetHook(UserInterface& ui)
{
  (void)ui;
  Halt();
}

void ChannelAccessReadInstruction::HaltImpl()
{
  m_channel_name = "";
  m_var_field_name = "";
  m_var_type = sup::dto::EmptyType;
  m_finish = 0;
  m_pv.reset();
}

} // namespace oac_tree

} // namespace sup
