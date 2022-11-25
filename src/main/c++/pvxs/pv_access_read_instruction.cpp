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

#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
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
const std::string OUTPUT_ATTRIBUTE_NAME = "output";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";

static bool _pv_access_read_instruction_initialised_flag =
  RegisterGlobalInstruction<PvAccessReadInstruction>();

PvAccessReadInstruction::PvAccessReadInstruction()
  : Instruction(PvAccessReadInstruction::Type)
{}

PvAccessReadInstruction::~PvAccessReadInstruction() = default;

bool PvAccessReadInstruction::SetupImpl(const Procedure& proc)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    return false;
  }
  if (!HasAttribute(OUTPUT_ATTRIBUTE_NAME))
  {
    return false;
  }
  auto var_names = proc.VariableNames();
  if (std::find(var_names.begin(), var_names.end(), GetAttribute(OUTPUT_ATTRIBUTE_NAME))
      == var_names.end())
  {
    return false;
  }
  return true;
}

ExecutionStatus PvAccessReadInstruction::ExecuteSingleImpl(UserInterface* ui, Workspace* ws)
{
  (void)ui;
  auto channel_name = GetAttribute(CHANNEL_ATTRIBUTE_NAME);
  sup::epics::PvAccessClientPV pv(channel_name);
  auto timeout_str = HasAttribute(TIMEOUT_ATTRIBUTE_NAME) ? GetAttribute(TIMEOUT_ATTRIBUTE_NAME)
                                                          : "-1.0";
  auto timeout = pv_access_helper::ParseTimeoutString(timeout_str);
  if (timeout >= 0.0 && !pv.WaitForValidValue(timeout))
  {
    return ExecutionStatus::FAILURE;
  }
  auto value = pv.GetValue();
  if (sup::dto::IsEmptyValue(value))
  {
    return ExecutionStatus::FAILURE;
  }
  if (!ws->SetValue(GetAttribute(OUTPUT_ATTRIBUTE_NAME), value))
  {
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

} // namespace sequencer

} // namespace sup
