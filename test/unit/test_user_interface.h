/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : UserInterface implementation
 *
 * Author        : B.Bauvir (IO)
 *
 * Copyright (c) : 2010-2019 ITER Organization,
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

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_TEST_USER_INTERFACE_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_TEST_USER_INTERFACE_H_

#include <sup/oac-tree/user_interface.h>

#include <utility>
#include <vector>

namespace sup {

namespace oac_tree {

namespace unit_test_helper {

using NullUserInterface = DefaultUserInterface;

class LogUserInterface : public DefaultUserInterface
{
public:
  using LogEntry = std::pair<int, std::string>;

  LogUserInterface();
  ~LogUserInterface();

  void Log(int severity, const std::string& message) override;

  std::string GetFullLog() const;

  std::vector<LogEntry> m_log_entries;
};

} // namespace unit_test_helper

} // namespace oac_tree

} // namespace sup

#endif // SUP_OAC_TREE_PLUGIN_EPICS_TEST_USER_INTERFACE_H_
