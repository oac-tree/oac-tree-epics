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
#include <sup/sequencer/instruction_utils.h>
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
const std::string OUTPUT_ATTRIBUTE_NAME = "outputVar";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";

static bool _ca_read_instruction_initialised_flag =
  RegisterGlobalInstruction<ChannelAccessReadInstruction>();

ChannelAccessReadInstruction::ChannelAccessReadInstruction()
  : Instruction(ChannelAccessReadInstruction::Type)
{
  AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
  AddAttributeDefinition(OUTPUT_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName).SetMandatory();
  AddAttributeDefinition(TIMEOUT_ATTRIBUTE_NAME, sup::dto::Float64Type)
    .SetCategory(AttributeCategory::kBoth);
}

ChannelAccessReadInstruction::~ChannelAccessReadInstruction() = default;

ExecutionStatus ChannelAccessReadInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  sup::dto::AnyValue value;
  if (!GetAttributeValue(OUTPUT_ATTRIBUTE_NAME, ws, ui, value))
  {
    return ExecutionStatus::FAILURE;
  }
  auto var_field_name = GetAttributeString(OUTPUT_ATTRIBUTE_NAME);
  std::string channel_name;
  if (!GetAttributeValueAs(CHANNEL_ATTRIBUTE_NAME, ws, ui, channel_name))
  {
    return ExecutionStatus::FAILURE;
  }
  auto channel_type = channel_access_helper::ChannelType(value.GetType());
  if (sup::dto::IsEmptyType(channel_type))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "type of variable field with name [" + var_field_name + "] is Empty";
    ui.LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
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
    ui.LogError(error_message);
    return ExecutionStatus::FAILURE;
  }
  if (!pv.WaitForValidValue(timeout_sec))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "channel with name [" + channel_name + "] timed out";
    ui.LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto ext_val = pv.GetExtendedValue();
  auto var_val = channel_access_helper::ConvertToTypedAnyValue(ext_val, value.GetType());
  if (sup::dto::IsEmptyValue(var_val))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "could not convert value from channel [" + channel_name +
      "] to type of variable field with name [" + var_field_name + "]";
    ui.LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!SetValueFromAttributeName(*this, ws, ui, OUTPUT_ATTRIBUTE_NAME, var_val))
  {
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

} // namespace sequencer

} // namespace sup
