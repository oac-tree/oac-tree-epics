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

#include "pv_access_write_instruction.h"

#include "pv_access_helper.h"

#include <sup/oac-tree/constants.h>
#include <sup/oac-tree/concrete_constraints.h>
#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/instruction_registry.h>
#include <sup/oac-tree/procedure.h>
#include <sup/oac-tree/user_interface.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/anyvalue_helper.h>
#include <sup/epics/pv_access_client_pv.h>

#include <algorithm>
#include <cmath>

namespace sup {

namespace oac_tree {

const std::string PvAccessWriteInstruction::Type = "PvAccessWrite";

static bool _pv_access_write_instruction_initialised_flag =
  RegisterGlobalInstruction<PvAccessWriteInstruction>();

PvAccessWriteInstruction::PvAccessWriteInstruction()
  : Instruction(PvAccessWriteInstruction::Type)
{
  AddAttributeDefinition(pv_access_helper::CHANNEL_ATTRIBUTE_NAME)
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

PvAccessWriteInstruction::~PvAccessWriteInstruction() = default;

ExecutionStatus PvAccessWriteInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  auto value = pv_access_helper::PackIntoStructIfScalar(GetNewValue(ui, ws));
  if (sup::dto::IsEmptyValue(value))
  {
    return ExecutionStatus::FAILURE;
  }
  std::string channel_name;
  if (!GetAttributeValueAs(pv_access_helper::CHANNEL_ATTRIBUTE_NAME, ws, ui, channel_name))
  {
    return ExecutionStatus::FAILURE;
  }
  sup::epics::PvAccessClientPV pv(channel_name);
  sup::dto::float64 timeout_sec = pv_access_helper::DEFAULT_TIMEOUT_SEC;
  if (!GetAttributeValueAs(Constants::TIMEOUT_SEC_ATTRIBUTE_NAME, ws, ui, timeout_sec))
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
      std::string warning_message =
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
