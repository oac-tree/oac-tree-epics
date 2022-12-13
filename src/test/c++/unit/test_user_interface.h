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
 * Copyright (c) : 2010-2019 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
******************************************************************************/

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_TEST_USER_INTERFACE_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_TEST_USER_INTERFACE_H_

#include <sup/sequencer/user_interface.h>

#include <utility>
#include <vector>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

class NullUserInterface : public UserInterface
{
public:
  NullUserInterface();
  ~NullUserInterface();

  void UpdateInstructionStatusImpl(const Instruction* instruction) override;
};

class LogUserInterface : public UserInterface
{
public:
  using LogEntry = std::pair<int, std::string>;

  LogUserInterface();
  ~LogUserInterface();

  void UpdateInstructionStatusImpl(const Instruction* instruction) override;
  void LogImpl(int severity, const std::string& message) override;

  std::vector<LogEntry> m_log_entries;
};

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup

#endif // SUP_SEQUENCER_PLUGIN_EPICS_TEST_USER_INTERFACE_H_

