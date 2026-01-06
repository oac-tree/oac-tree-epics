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
* Copyright (c) : 2010-2026 ITER Organization,
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

#include "pv_access_write_instruction.h"

#include "pv_access_helper.h"

#include <sup/oac-tree/constants.h>
#include <sup/oac-tree/concrete_constraints.h>
#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/generic_utils.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/instruction_utils.h>
#include <sup/oac-tree/procedure.h>
#include <sup/oac-tree/user_interface.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/anyvalue_helper.h>
#include <sup/epics/pv_access_client_pv.h>

namespace sup {

namespace oac_tree {

const std::string PvAccessWriteInstruction::Type = "PvAccessWrite";

static bool _pv_access_write_instruction_initialised_flag =
  RegisterGlobalInstruction<PvAccessWriteInstruction>();

PvAccessWriteInstruction::PvAccessWriteInstruction()
  : Instruction(PvAccessWriteInstruction::Type)
{
  (void)AddAttributeDefinition(pv_access_helper::CHANNEL_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
  (void)AddAttributeDefinition(Constants::GENERIC_VARIABLE_NAME_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kVariableName);
  (void)AddAttributeDefinition(Constants::TYPE_ATTRIBUTE_NAME);
  (void)AddAttributeDefinition(Constants::VALUE_ATTRIBUTE_NAME);
  (void)AddAttributeDefinition(Constants::TIMEOUT_SEC_ATTRIBUTE_NAME, sup::dto::Float64Type)
    .SetCategory(AttributeCategory::kBoth);
  AddConstraint(MakeConstraint<Xor>(
    MakeConstraint<Exists>(Constants::GENERIC_VARIABLE_NAME_ATTRIBUTE_NAME),
    MakeConstraint<And>(MakeConstraint<Exists>(Constants::TYPE_ATTRIBUTE_NAME),
                        MakeConstraint<Exists>(Constants::VALUE_ATTRIBUTE_NAME))));
}

PvAccessWriteInstruction::~PvAccessWriteInstruction() = default;

bool PvAccessWriteInstruction::InitHook(UserInterface& ui, Workspace& ws)
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

ExecutionStatus PvAccessWriteInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  auto value = pv_access_helper::PackIntoStructIfScalar(GetNewValue(ui, ws));
  if (sup::dto::IsEmptyValue(value))
  {
    return ExecutionStatus::FAILURE;
  }
  if (IsHaltRequested())
  {
    return ExecutionStatus::FAILURE;
  }
  auto now = utils::GetNanosecsSinceEpoch();
  if (!m_pv->IsConnected())
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
  if (!m_pv->SetValue(value))
  {
    auto json_value = sup::dto::ValuesToJSONString(value).substr(0, 1024);
    const std::string warning_message = InstructionWarningProlog(*this) +
      "could not write value [" + json_value + "] to channel [" + m_channel_name + "]";
    LogWarning(ui, warning_message);
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

void PvAccessWriteInstruction::ResetHook(UserInterface& ui)
{
  (void)ui;
  Halt();
}

void PvAccessWriteInstruction::HaltImpl()
{
  m_channel_name = "";
  m_finish = 0;
  m_pv.reset();
}

sup::dto::AnyValue PvAccessWriteInstruction::GetNewValue(UserInterface& ui, Workspace& ws) const
{
  if (HasAttribute(Constants::GENERIC_VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    sup::dto::AnyValue result;
    if (!GetAttributeValue(Constants::GENERIC_VARIABLE_NAME_ATTRIBUTE_NAME, ws, ui, result))
    {
      return {};
    }
    if (sup::dto::IsEmptyValue(result))
    {
      const std::string warning_message =
        InstructionWarningProlog(*this) + "value from field [" +
        GetAttributeString(Constants::GENERIC_VARIABLE_NAME_ATTRIBUTE_NAME) + "] is empty";
      LogWarning(ui, warning_message);
    }
    return result;
  }
  return ParseAnyValueAttributePair(*this, ws, ui, Constants::TYPE_ATTRIBUTE_NAME,
                                    Constants::VALUE_ATTRIBUTE_NAME);
}

} // namespace oac_tree

} // namespace sup
