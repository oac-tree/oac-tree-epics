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

#include "pv_access_read_instruction.h"

#include "pv_access_helper.h"

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/user_interface.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/json_value_parser.h>
#include <sup/epics/pv_access_client_pv.h>

#include <algorithm>
#include <cmath>

namespace sup {

namespace sequencer {

const std::string PvAccessReadInstruction::Type = "PvAccessRead";

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string OUTPUT_ATTRIBUTE_NAME = "outputVar";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";

static bool _pv_access_read_instruction_initialised_flag =
  RegisterGlobalInstruction<PvAccessReadInstruction>();

PvAccessReadInstruction::PvAccessReadInstruction()
  : Instruction(PvAccessReadInstruction::Type)
{
  AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
  AddAttributeDefinition(OUTPUT_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName).SetMandatory();
  AddAttributeDefinition(TIMEOUT_ATTRIBUTE_NAME, sup::dto::Float64Type)
    .SetCategory(AttributeCategory::kBoth);
}

PvAccessReadInstruction::~PvAccessReadInstruction() = default;

ExecutionStatus PvAccessReadInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  std::string channel_name;
  if (!GetAttributeValueAs(CHANNEL_ATTRIBUTE_NAME, ws, ui, channel_name))
  {
    return ExecutionStatus::FAILURE;
  }
  sup::epics::PvAccessClientPV pv(channel_name);
  sup::dto::float64 timeout_sec = pv_access_helper::DEFAULT_TIMEOUT_SEC;
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
  if (!pv.WaitForValidValue(timeout_sec))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "channel with name [" + channel_name + "] timed out";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto value = pv.GetValue();
  if (sup::dto::IsEmptyValue(value))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "value retrieved from channel [" + channel_name + "] is empty";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!SetValueFromAttributeName(*this, ws, ui, OUTPUT_ATTRIBUTE_NAME, value))
  {
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

} // namespace sequencer

} // namespace sup
