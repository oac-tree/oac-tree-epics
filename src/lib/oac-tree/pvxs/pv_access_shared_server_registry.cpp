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

#include "pv_access_shared_server_registry.h"

#include <sup/oac-tree/exceptions.h>

namespace sup
{
namespace oac_tree
{
PvAccessSharedServerRegistry::PvAccessSharedServerRegistry()
  : m_servers{}
{}

PvAccessSharedServerRegistry::~PvAccessSharedServerRegistry() = default;

PvAccessSharedServer& PvAccessSharedServerRegistry::GetServer(const Workspace* ws)
{
  auto iter = m_servers.find(ws);
  if (iter == m_servers.end())
  {
    auto result = m_servers.emplace(ws, std::make_unique<PvAccessSharedServer>());
    if (!result.second)
    {
      throw InvalidOperationException("Failed to create PvAccessSharedServer");
    }
    iter = result.first;
  }
  return *iter->second;
}

void PvAccessSharedServerRegistry::Setup(const Workspace* ws)
{
  auto iter = m_servers.find(ws);
  if (iter == m_servers.end())
  {
    throw InvalidOperationException("Trying to setup unknown PvAccessSharedServer");
  }
  iter->second->Setup();
}

void PvAccessSharedServerRegistry::Teardown(const Workspace* ws)
{
  auto iter = m_servers.find(ws);
  if (iter == m_servers.end())
  {
    throw InvalidOperationException("Trying to setup unknown PvAccessSharedServer");
  }
  iter->second->Teardown();
  (void)m_servers.erase(iter);
}

}  // namespace oac_tree

}  // namespace sup
