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
  , m_timeout_sec{pv_access_helper::DEFAULT_TIMEOUT_SEC}
{
  AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
  AddAttributeDefinition(OUTPUT_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
  AddAttributeDefinition(TIMEOUT_ATTRIBUTE_NAME, sup::dto::Float64Type);
}

PvAccessReadInstruction::~PvAccessReadInstruction() = default;

void PvAccessReadInstruction::SetupImpl(const Procedure&)
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

void PvAccessReadInstruction::ResetHook()
{
  m_timeout_sec = pv_access_helper::DEFAULT_TIMEOUT_SEC;
}

ExecutionStatus PvAccessReadInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  auto channel_name = GetAttributeValue<std::string>(CHANNEL_ATTRIBUTE_NAME);
  sup::epics::PvAccessClientPV pv(channel_name);
  if (!pv.WaitForValidValue(m_timeout_sec))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "channel with name [" + channel_name + "] timed out";
    ui.LogWarning(warning_message);
    return ExecutionStatus::FAILURE;
  }
  auto value = pv.GetValue();
  if (sup::dto::IsEmptyValue(value))
  {
    std::string warning_message = InstructionWarningProlog(*this) +
      "value retrieved from channel [" + channel_name + "] is empty";
    ui.LogWarning(warning_message);
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
