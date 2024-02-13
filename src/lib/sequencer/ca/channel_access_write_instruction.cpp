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

#include "channel_access_write_instruction.h"
#include "channel_access_helper.h"

#include <sup/sequencer/concrete_constraints.h>
#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/user_interface.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/anyvalue_helper.h>
#include <sup/epics/channel_access_pv.h>

#include <algorithm>
#include <cmath>

namespace sup {

namespace sequencer {

const std::string ChannelAccessWriteInstruction::Type = "ChannelAccessWrite";

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string VARIABLE_NAME_ATTRIBUTE_NAME = "varName";
const std::string TYPE_ATTRIBUTE_NAME = "type";
const std::string VALUE_ATTRIBUTE_NAME = "value";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";

static bool _ca_write_instruction_initialised_flag =
   RegisterGlobalInstruction<ChannelAccessWriteInstruction>();

ChannelAccessWriteInstruction::ChannelAccessWriteInstruction()
  : Instruction(ChannelAccessWriteInstruction::Type)
{
  AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
  AddAttributeDefinition(VARIABLE_NAME_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName);
  AddAttributeDefinition(TYPE_ATTRIBUTE_NAME);
  AddAttributeDefinition(VALUE_ATTRIBUTE_NAME);
  AddAttributeDefinition(TIMEOUT_ATTRIBUTE_NAME, sup::dto::Float64Type)
    .SetCategory(AttributeCategory::kBoth);
  AddConstraint(MakeConstraint<Xor>(
    MakeConstraint<Exists>(VARIABLE_NAME_ATTRIBUTE_NAME),
    MakeConstraint<And>(MakeConstraint<Exists>(TYPE_ATTRIBUTE_NAME),
                        MakeConstraint<Exists>(VALUE_ATTRIBUTE_NAME))));
}

ChannelAccessWriteInstruction::~ChannelAccessWriteInstruction() = default;

ExecutionStatus ChannelAccessWriteInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  (void)ui;
  auto value = channel_access_helper::ExtractChannelValue(GetNewValue(ui, ws));
  if (sup::dto::IsEmptyValue(value))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "value to write is Empty";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  std::string channel_name;
  if (!GetAttributeValueAs(CHANNEL_ATTRIBUTE_NAME, ws, ui, channel_name))
  {
    return ExecutionStatus::FAILURE;
  }
  auto channel_type = value.GetType();
  sup::epics::ChannelAccessPV pv(channel_name, channel_type);
  sup::dto::float64 timeout_sec = channel_access_helper::DEFAULT_TIMEOUT_SEC;
  if (!GetAttributeValueAs(TIMEOUT_ATTRIBUTE_NAME, ws, ui, timeout_sec))
  {
    return ExecutionStatus::FAILURE;
  }
  if (timeout_sec < 0)
  {
    std::string error_message = InstructionSetupExceptionProlog(*this) +
      "timeout attribute is not positive: " + std::to_string(timeout_sec);
    LogError(ui, error_message);
    return ExecutionStatus::FAILURE;
  }
  if (!pv.WaitForConnected(timeout_sec))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "channel with name [" + channel_name + "] timed out";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!pv.SetValue(value))
  {
    auto json_value = sup::dto::ValuesToJSONString(value).substr(0, 1024);
    std::string warning_message = InstructionWarningProlog(*this) +
      "could not write value [" + json_value + "] to channel [" + channel_name + "]";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

sup::dto::AnyValue ChannelAccessWriteInstruction::GetNewValue(UserInterface& ui,
                                                              Workspace& ws) const
{
  if (HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    sup::dto::AnyValue result;
    if (!GetAttributeValue(VARIABLE_NAME_ATTRIBUTE_NAME, ws, ui, result))
    {
      return {};
    }
    return result;
  }
  return ParseAnyValueAttributePair(*this, ws, ui, TYPE_ATTRIBUTE_NAME, VALUE_ATTRIBUTE_NAME);
}

} // namespace sequencer

} // namespace sup
