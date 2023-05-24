/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP - Sequencer
 *
 * Description   : Sequencer for operational procedures
 *
 * Author        : Walter Van Herck (IO)
 *
 * Copyright (c) : 2010-2022 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
******************************************************************************/

#include "system_call_instruction.h"

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction_registry.h>

#include <cstdlib>

namespace sup {

namespace sequencer {

const std::string SystemCallInstruction::Type = "SystemCall";
static bool _systemcall_initialised_flag = RegisterGlobalInstruction<SystemCallInstruction>();

const std::string COMMAND_ATTRIBUTE_NAME = "command";

SystemCallInstruction::SystemCallInstruction()
  : Instruction(Type)
{
  AddAttributeDefinition(COMMAND_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
}

SystemCallInstruction::~SystemCallInstruction() = default;

ExecutionStatus SystemCallInstruction::ExecuteSingleImpl(UserInterface& ui, Workspace& ws)
{
  (void)ui;
  (void)ws;
  auto command = GetAttributeValue<std::string>(COMMAND_ATTRIBUTE_NAME);
  auto exit_status = std::system(command.c_str());
  return exit_status == 0 ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE;
}

} // namespace sequencer

} // namespace sup
