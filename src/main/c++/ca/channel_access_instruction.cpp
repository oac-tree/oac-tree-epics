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

#include "channel_access_instruction.h"
#include "channel_access_helper.h"

#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue.h>
#include <sup/dto/json_value_parser.h>
#include <sup/epics/channel_access_pv.h>

#include <algorithm>
#include <cmath>

namespace sup {

namespace sequencer {

const std::string ChannelAccessReadInstruction::Type = "ChannelAccessRead";
// const std::string ChannelAccessWriteInstruction::Type = "ChannelAccessWrite";

const long DEFAULT_TIMEOUT_SEC = 2.0;

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string VARIABLE_NAME_ATTRIBUTE_NAME = "varName";
const std::string TIMEOUT_ATTRIBUTE_NAME = "timeout";

static bool _caclient_initialised_flag =
  (RegisterGlobalInstruction<ChannelAccessReadInstruction>()); // &&
  //  RegisterGlobalInstruction<ChannelAccessWriteInstruction>());

ChannelAccessReadInstruction::ChannelAccessReadInstruction()
  : Instruction(ChannelAccessReadInstruction::Type)
  , m_timeout_sec{DEFAULT_TIMEOUT_SEC}
{}

ChannelAccessReadInstruction::~ChannelAccessReadInstruction() = default;

bool ChannelAccessReadInstruction::SetupImpl(const Procedure& proc)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME) || !HasAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
  {
    return false;
  }
  auto var_names = proc.VariableNames();
  if (std::find(var_names.begin(), var_names.end(), GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME))
      == var_names.end())
  {
    return false;
  }
  if (HasAttribute(TIMEOUT_ATTRIBUTE_NAME))
  {
    sup::dto::JSONAnyValueParser parser;
    if (!parser.TypedParseString(sup::dto::Float64Type, GetAttribute(TIMEOUT_ATTRIBUTE_NAME)))
    {
      return false;
    }
    auto timeout_val = parser.MoveAnyValue().As<sup::dto::float64>();
    if (timeout_val < 0)
    {
      return false;
    }
    m_timeout_sec = timeout_val;
  }
  return true;
}

void ChannelAccessReadInstruction::ResetHook()
{
  m_timeout_sec = DEFAULT_TIMEOUT_SEC;
}

ExecutionStatus ChannelAccessReadInstruction::ExecuteSingleImpl(UserInterface*, Workspace* ws)
{
  sup::dto::AnyValue value;
  if (!ws->GetValue(GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME), value))
  {
    return ExecutionStatus::FAILURE;
  }
  sup::epics::ChannelAccessPV pv(GetAttribute(CHANNEL_ATTRIBUTE_NAME),
                                 channel_access_helper::ChannelType(value.GetType()));
  if (!pv.WaitForConnected(m_timeout_sec))
  {
    return ExecutionStatus::FAILURE;
  }
  auto ext_val = pv.GetExtendedValue();
  auto var_val = channel_access_helper::ConvertToTypedAnyValue(ext_val, value.GetType());
  if (sup::dto::IsEmptyValue(var_val))
  {
    return ExecutionStatus::FAILURE;
  }
  if (!ws->SetValue(GetAttribute(VARIABLE_NAME_ATTRIBUTE_NAME), var_val))
  {
    return ExecutionStatus::FAILURE;
  }
  return ExecutionStatus::SUCCESS;
}

// ChannelAccessWriteInstruction::ChannelAccessWriteInstruction()
//   : Instruction(ChannelAccessWriteInstruction::Type)
// {}

// ChannelAccessWriteInstruction::~ChannelAccessWriteInstruction() = default;

// bool ChannelAccessWriteInstruction::VerifyAttributes (const Procedure& proc) const
// {
//   bool status = Instruction::HasAttribute("channel");
//   if (status)
//     {
//       log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - Method called with channel '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
//       status = ((Instruction::HasAttribute("variable") && (proc.VariableNames().end() != std::find(proc.VariableNames().begin(), proc.VariableNames().end(), Instruction::GetAttribute("variable").c_str()))) ||
//                 (Instruction::HasAttribute("datatype") && Instruction::HasAttribute("instance")));
//     }
//   return status;
// }

// bool ChannelAccessWriteInstruction::SetupImpl (const Procedure& proc)
// {
//   log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - Method called ..", Instruction::GetName().c_str());
//   bool status = VerifyAttributes(proc);
//   if (status)
//     {
//       if (Instruction::HasAttribute("variable"))
//         { // Read variable to allow verifying the type
//           log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - .. using workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());
//           status = proc.GetVariableValue(Instruction::GetAttribute("variable"), _value);
//         }
//       else
//         {
//           log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - .. using type '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("datatype").c_str());
//           log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - .. and instance '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("instance").c_str());
//           _value = ccs::types::AnyValue (Instruction::GetAttribute("datatype").c_str());
//           status = _value.ParseInstance(Instruction::GetAttribute("instance").c_str());
//         }
//     }
//   if (status)
//     {
//       status = ChannelAccessInstructionHelper::VerifyValue(_value);
//     }
// #ifdef LOG_DEBUG_ENABLE
//   if (!status)
//     {
//       log_error("ChannelAccessWriteInstruction('%s')::SetupImpl - .. failure", Instruction::GetName().c_str());
//     }
// #endif
//   if (status && Instruction::HasAttribute("delay"))
//     {
//       status = ChannelAccessInstructionHelper::SetDelay(Instruction::GetAttribute("delay").c_str());
//     }
//   return status;
// }

// ExecutionStatus ChannelAccessWriteInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
// {
//   log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Method called ..", Instruction::GetName().c_str());
//   bool status = static_cast<bool>(_value.GetType());
//   if (status)
//     { // Attach to CA variable
//       log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Connect ..", Instruction::GetName().c_str());
//       status = ChannelAccessInstructionHelper::HandleConnect(Instruction::GetAttribute("channel").c_str());
//     }
//   if (status && Instruction::HasAttribute("variable"))
//     { // Update from workspace variable that may have changed since setup
//       log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Use workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());
//       status = ws->GetValue(Instruction::GetAttribute("variable"), _value);
//     }
//   if (status)
//     {
//       log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Write ..", Instruction::GetName().c_str());
//       status = ChannelAccessInstructionHelper::WriteChannel(_value);
//     }
//   // Detach from CA variable
//   log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Detach ..", Instruction::GetName().c_str());
//   (void)ChannelAccessInstructionHelper::HandleDetach();
//   return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);
// }

} // namespace sequencer

} // namespace sup
