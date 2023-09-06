/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : UserInterface implementation
*
* Author        : B.Bauvir (IO)
*
* Copyright (c) : 2010-2020 ITER Organization,
*                 CS 90 046
*                 13067 St. Paul-lez-Durance Cedex
*                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

#include "test_user_interface.h"

#include <sstream>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

LogUserInterface::LogUserInterface()
  : m_log_entries{}
{}

LogUserInterface::~LogUserInterface() = default;

void LogUserInterface::Log(int severity, const std::string& message)
{
  m_log_entries.emplace_back(severity, message);
}

std::string LogUserInterface::GetFullLog() const
{
  std::ostringstream oss;
  for (const auto& log_entry : m_log_entries)
  {
    oss << "Severity(" << log_entry.first << "): " << log_entry.second << std::endl;
  }
  return oss.str();
}

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup
