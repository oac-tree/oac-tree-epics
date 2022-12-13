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

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/instruction_registry.h>
#include <sup/sequencer/log_severity.h>
#include <sup/sequencer/user_interface.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue_helper.h>

#include <functional>
#include <map>
#include <sstream>

namespace
{
int SeverityFromString(const std::string& level);
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

void LogInstruction::SetupImpl(const Procedure&)
{
  if (!HasAttribute(MESSAGE_ATTRIBUTE_NAME) && !HasAttribute(INPUT_ATTRIBUTE_NAME))
  {
    std::string error_message = InstructionSetupExceptionProlog() +
      "instruction requires at least one of the attributes [" + MESSAGE_ATTRIBUTE_NAME + ", " +
      INPUT_ATTRIBUTE_NAME + "]";
    throw InstructionSetupException(error_message);
  }
}

ExecutionStatus LogInstruction::ExecuteSingleImpl(UserInterface* ui, Workspace* ws)
{
  int severity = log::SUP_SEQ_LOG_INFO;  // Default severity if not explicitly specified
  if (HasAttribute(SEVERITY_ATTRIBUTE_NAME))
  {
    auto severity_str = GetAttribute(SEVERITY_ATTRIBUTE_NAME);
    severity = SeverityFromString(severity_str);
    if (severity < 0)
    {
      std::string error_message = InstructionErrorLogProlog() +
        "could not parse severity [" + severity_str + "] as valid severity level";
      ui->LogError(error_message);
      return ExecutionStatus::FAILURE;
    }
  }
  std::ostringstream oss;
  if (HasAttribute(MESSAGE_ATTRIBUTE_NAME))
  {
    oss << GetAttribute(MESSAGE_ATTRIBUTE_NAME);
  }
  if (HasAttribute(INPUT_ATTRIBUTE_NAME))
  {
    auto input_field_name = GetAttribute(INPUT_ATTRIBUTE_NAME);
    auto input_var_name = SplitFieldName(input_field_name).first;
    if (!ws->HasVariable(input_var_name))
    {
      std::string error_message = InstructionErrorLogProlog() +
        "workspace does not contain input variable with name [" + input_var_name + "]";
      ui->LogError(error_message);
      return ExecutionStatus::FAILURE;
    }
    sup::dto::AnyValue value;
    if (!ws->GetValue(GetAttribute(INPUT_ATTRIBUTE_NAME), value))
    {
      std::string warning_message = InstructionWarningLogProlog() +
        "could not read variable field with name [" + input_var_name + "] from workspace";
      ui->LogWarning(warning_message);
      return ExecutionStatus::FAILURE;
    }
    oss << sup::dto::ValuesToJSONString(value);
  }
  ui->Log(severity, oss.str());
  return ExecutionStatus::SUCCESS;
}

} // namespace sequencer

} // namespace sup

namespace
{
int SeverityFromString(const std::string& level)
{
  static const std::map<std::string, int> string_severity_map = {
      {"emergency", sup::sequencer::log::SUP_SEQ_LOG_EMERG},
      {"alert", sup::sequencer::log::SUP_SEQ_LOG_ALERT},
      {"critical", sup::sequencer::log::SUP_SEQ_LOG_CRIT},
      {"error", sup::sequencer::log::SUP_SEQ_LOG_ERR},
      {"warning", sup::sequencer::log::SUP_SEQ_LOG_WARNING},
      {"notice", sup::sequencer::log::SUP_SEQ_LOG_NOTICE},
      {"info", sup::sequencer::log::SUP_SEQ_LOG_INFO},
      {"debug", sup::sequencer::log::SUP_SEQ_LOG_DEBUG},
      {"trace", sup::sequencer::log::SUP_SEQ_LOG_TRACE},
    };
  auto it = string_severity_map.find(level);
  if (it == string_severity_map.end())
  {
    return -1;
  }
  return it->second;
}

}  // unnamed namespace
