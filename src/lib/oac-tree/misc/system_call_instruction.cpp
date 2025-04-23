/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP - oac-tree
 *
 * Description   : oac-tree for operational procedures
 *
 * Author        : Walter Van Herck (IO)
 *
 * Copyright (c) : 2010-2025 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 * SPDX-License-Identifier: MIT
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file LICENSE located in the top level directory
 * of the distribution package.
******************************************************************************/

#include "system_call_instruction.h"

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/instruction_registry.h>

#include <cstdlib>

namespace sup {

namespace oac_tree {

const std::string SystemCallInstruction::Type = "SystemCall";
static bool _systemcall_initialised_flag = RegisterGlobalInstruction<SystemCallInstruction>();

const std::string COMMAND_ATTRIBUTE_NAME = "command";

SystemCallInstruction::SystemCallInstruction()
  : Instruction(Type)
{
  AddAttributeDefinition(COMMAND_ATTRIBUTE_NAME)
    .SetCategory(AttributeCategory::kBoth).SetMandatory();
}

SystemCallInstruction::~SystemCallInstruction() = default;

ExecutionStatus SystemCallInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  std::string command;
  if (!GetAttributeValueAs(COMMAND_ATTRIBUTE_NAME, ws, ui, command))
  {
    return ExecutionStatus::FAILURE;
  }
  auto exit_status = std::system(command.c_str());
  return exit_status == 0 ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE;
}

} // namespace oac_tree

} // namespace sup
