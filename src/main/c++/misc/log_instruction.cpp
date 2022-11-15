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

#include "log_instruction.h"

#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/log.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue_helper.h>

#include <functional>
#include <map>
#include <sstream>

namespace
{
void LogWrapper(const std::string& level, const std::string& message);
}  // unnamed namespace

namespace sup {

namespace sequencer {

const std::string LogInstruction::Type = "Log";
static bool _log_initialised_flag = RegisterGlobalInstruction<LogInstruction>();

const std::string LOG_SOURCE = "sup::sequencer";

const std::string MESSAGE_ATTRIBUTE_NAME = "message";
const std::string INPUT_ATTRIBUTE_NAME = "input";
const std::string SEVERITY_ATTRIBUTE_NAME = "severity";

LogInstruction::LogInstruction()
  : Instruction(Type)
{}

LogInstruction::~LogInstruction() = default;

bool LogInstruction::SetupImpl(const Procedure& proc)
{
  (void)proc;
  return HasAttribute(MESSAGE_ATTRIBUTE_NAME) || HasAttribute(INPUT_ATTRIBUTE_NAME);
}

ExecutionStatus LogInstruction::ExecuteSingleImpl(UserInterface*, Workspace* ws)
{
  std::string severity_str;
  // if this string is empty or does not correspond to a known severity level, 'info' level is used.
  if (HasAttribute(SEVERITY_ATTRIBUTE_NAME))
  {
    severity_str = GetAttribute(SEVERITY_ATTRIBUTE_NAME);
  }
  std::ostringstream oss;
  if (HasAttribute(MESSAGE_ATTRIBUTE_NAME))
  {
    oss << GetAttribute(MESSAGE_ATTRIBUTE_NAME);
  }
  if (HasAttribute(INPUT_ATTRIBUTE_NAME))
  {
    sup::dto::AnyValue value;
    if (!ws->GetValue(GetAttribute(INPUT_ATTRIBUTE_NAME), value))
    {
      return ExecutionStatus::FAILURE;
    }
    oss << sup::dto::ValuesToJSONString(value);
  }
  LogWrapper(severity_str, oss.str());
  return ExecutionStatus::SUCCESS;
}

} // namespace sequencer

} // namespace sup

namespace
{
void LogWrapper(const std::string& level, const std::string& message)
{
  static const std::map<std::string, std::function<void(const std::string&, const std::string&)>>
    log_function_map = {
      {"emergency", sup::sequencer::log::SimpleEmergency},
      {"alert", sup::sequencer::log::SimpleAlert},
      {"critical", sup::sequencer::log::SimpleCritical},
      {"error", sup::sequencer::log::SimpleError},
      {"warning", sup::sequencer::log::SimpleWarning},
      {"notice", sup::sequencer::log::SimpleNotice},
      {"info", sup::sequencer::log::SimpleInfo},
      {"debug", sup::sequencer::log::SimpleDebug},
      {"trace", sup::sequencer::log::SimpleTrace},
    };
  auto it = log_function_map.find(level);
  if (it == log_function_map.end())
  {
    sup::sequencer::log::SimpleInfo(sup::sequencer::LOG_SOURCE, message);
  }
  else
  {
    it->second(sup::sequencer::LOG_SOURCE, message);
  }
}
}  // unnamed namespace
