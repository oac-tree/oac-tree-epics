/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) oac-tree component
*
* Description   : Instruction node implementation
*
* Author        : Walter Van Herck (IO)
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

#include "pv_access_read_instruction.h"

#include "pv_access_helper.h"

#include <sup/oac-tree/constants.h>
#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/generic_utils.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/instruction_utils.h>
#include <sup/oac-tree/user_interface.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/epics/pv_access_client_pv.h>

namespace sup {

namespace oac_tree {

const std::string PvAccessReadInstruction::Type = "PvAccessRead";

static bool _pv_access_read_instruction_initialised_flag =
  RegisterGlobalInstruction<PvAccessReadInstruction>();

PvAccessReadInstruction::PvAccessReadInstruction()
  : Instruction(PvAccessReadInstruction::Type)
  , m_channel_name{}
  , m_finish{}
  , m_pv{}
{
  (void)AddAttributeDefinition(pv_access_helper::CHANNEL_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
  (void)AddAttributeDefinition(Constants::OUTPUT_VARIABLE_NAME_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName).SetMandatory();
  (void)AddAttributeDefinition(Constants::TIMEOUT_SEC_ATTRIBUTE_NAME, sup::dto::Float64Type)
    .SetCategory(AttributeCategory::kBoth);
}

PvAccessReadInstruction::~PvAccessReadInstruction() = default;

bool PvAccessReadInstruction::InitHook(UserInterface& ui, Workspace& ws)
{
  if (!GetAttributeValueAs(pv_access_helper::CHANNEL_ATTRIBUTE_NAME, ws, ui, m_channel_name))
  {
    return false;
  }
  sup::dto::uint64 timeout_ns = pv_access_helper::DEFAULT_TIMEOUT_NS;
  if (!instruction_utils::GetVariableTimeoutAttribute(
            *this, ui, ws, Constants::TIMEOUT_SEC_ATTRIBUTE_NAME, timeout_ns))
  {
    return false;
  }
  m_finish = utils::GetNanosecsSinceEpoch() + timeout_ns;
  m_pv = std::make_unique<sup::epics::PvAccessClientPV>(m_channel_name);
  return true;
}

ExecutionStatus PvAccessReadInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
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
    const std::string warning_message = InstructionWarningProlog(*this) +
      "channel with name [" + m_channel_name + "] timed out";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  if (!SetValueFromAttributeName(*this, ws, ui, Constants::OUTPUT_VARIABLE_NAME_ATTRIBUTE_NAME, ext_val.value))
  {
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

void PvAccessReadInstruction::ResetHook(UserInterface& ui)
{
  (void)ui;
  Halt();
}

void PvAccessReadInstruction::HaltImpl()
{
  m_channel_name = "";
  m_finish = 0;
  m_pv.reset();
}

} // namespace oac_tree

} // namespace sup
