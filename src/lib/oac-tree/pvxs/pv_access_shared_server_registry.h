/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Walter Van Herck
 *
 * Copyright (c) : 2010-2026 ITER Organization,
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

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_SHARED_SERVER_REGISTRY_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_SHARED_SERVER_REGISTRY_H_

#include "pv_access_shared_server.h"

#include <sup/oac-tree/workspace.h>

#include <map>
#include <memory>

namespace sup
{
namespace oac_tree
{
class PvAccessSharedServerRegistry
{
public:
  PvAccessSharedServerRegistry();
  PvAccessSharedServerRegistry(const PvAccessSharedServerRegistry&) = delete;
  PvAccessSharedServerRegistry(PvAccessSharedServerRegistry&&) = delete;
  PvAccessSharedServerRegistry& operator=(const PvAccessSharedServerRegistry&) = delete;
  PvAccessSharedServerRegistry& operator=(PvAccessSharedServerRegistry&&) = delete;
  ~PvAccessSharedServerRegistry();

  PvAccessSharedServer& GetServer(const Workspace* ws);

  void Setup(const Workspace* ws);

  void Teardown(const Workspace* ws);

private:
  std::map<const Workspace*, std::unique_ptr<PvAccessSharedServer>> m_servers;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_SHARED_SERVER_REGISTRY_H_
