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

#include "channel_access_write_instruction.h"
#include "channel_access_helper.h"

#include <sup/oac-tree/constants.h>
#include <sup/oac-tree/concrete_constraints.h>
#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/generic_utils.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/instruction_utils.h>
#include <sup/oac-tree/user_interface.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/anyvalue_helper.h>
#include <sup/epics/channel_access_pv.h>

#include <algorithm>
#include <cmath>

namespace sup {

namespace oac_tree {

const std::string ChannelAccessWriteInstruction::Type = "ChannelAccessWrite";

static bool _ca_write_instruction_initialised_flag =
   RegisterGlobalInstruction<ChannelAccessWriteInstruction>();

ChannelAccessWriteInstruction::ChannelAccessWriteInstruction()
  : Instruction(ChannelAccessWriteInstruction::Type)
  , m_channel_name{}
  , m_value{}
  , m_finish{}
  , m_pv{}
{
  AddAttributeDefinition(channel_access_helper::CHANNEL_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
  AddAttributeDefinition(Constants::GENERIC_VARIABLE_NAME_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName);
  AddAttributeDefinition(Constants::TYPE_ATTRIBUTE_NAME);
  AddAttributeDefinition(Constants::VALUE_ATTRIBUTE_NAME);
  AddAttributeDefinition(Constants::TIMEOUT_SEC_ATTRIBUTE_NAME, sup::dto::Float64Type)
    .SetCategory(AttributeCategory::kBoth);
  AddConstraint(MakeConstraint<Xor>(
    MakeConstraint<Exists>(Constants::GENERIC_VARIABLE_NAME_ATTRIBUTE_NAME),
    MakeConstraint<And>(MakeConstraint<Exists>(Constants::TYPE_ATTRIBUTE_NAME),
                        MakeConstraint<Exists>(Constants::VALUE_ATTRIBUTE_NAME))));
}

ChannelAccessWriteInstruction::~ChannelAccessWriteInstruction() = default;

bool ChannelAccessWriteInstruction::InitHook(UserInterface& ui, Workspace& ws)
{
  if (!GetAttributeValueAs(channel_access_helper::CHANNEL_ATTRIBUTE_NAME, ws, ui, m_channel_name))
  {
    return false;
  }
  m_value = channel_access_helper::ExtractChannelValue(GetNewValue(ui, ws));
  if (sup::dto::IsEmptyValue(m_value))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "value to write is Empty";
    LogWarning(ui, warning_message);
    return false;
  }
  auto channel_type = m_value.GetType();
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

ExecutionStatus ChannelAccessWriteInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  if (IsHaltRequested())
  {
    return ExecutionStatus::FAILURE;
  }
  auto now = utils::GetNanosecsSinceEpoch();
  auto ext_val = m_pv->GetExtendedValue();
  if (!ext_val.connected)
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
  if (!m_pv->SetValue(m_value))
  {
    auto json_value = sup::dto::ValuesToJSONString(m_value).substr(0, 1024);
    std::string warning_message = InstructionWarningProlog(*this) +
      "could not write value [" + json_value + "] to channel [" + m_channel_name + "]";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

sup::dto::AnyValue ChannelAccessWriteInstruction::GetNewValue(UserInterface& ui,
                                                              Workspace& ws) const
{
  if (HasAttribute(Constants::GENERIC_VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    sup::dto::AnyValue result;
    if (!GetAttributeValue(Constants::GENERIC_VARIABLE_NAME_ATTRIBUTE_NAME, ws, ui, result))
    {
      return {};
    }
    return result;
  }
  return ParseAnyValueAttributePair(*this, ws, ui, Constants::TYPE_ATTRIBUTE_NAME, Constants::VALUE_ATTRIBUTE_NAME);
}

void ChannelAccessWriteInstruction::ResetHook(UserInterface& ui)
{
  (void)ui;
  Halt();
}

void ChannelAccessWriteInstruction::HaltImpl()
{
  m_channel_name = "";
  m_value = sup::dto::AnyValue{};
  m_finish = 0;
  m_pv.reset();
}

} // namespace oac_tree

} // namespace sup
