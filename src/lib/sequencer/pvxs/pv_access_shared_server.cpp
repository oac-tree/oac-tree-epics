/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Gennady Pospelov
 *
 * Copyright (c) : 2010-2025 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include "pv_access_shared_server.h"

#include <sup/epics/pv_access_server.h>

namespace sup
{
namespace sequencer
{

PvAccessSharedServer::PvAccessSharedServer()
  : m_server{}
{}

PvAccessSharedServer::~PvAccessSharedServer() = default;

void PvAccessSharedServer::AddVariable(const std::string& name, const sup::dto::AnyValue& start_val,
                                       VariableCallback cb)
{
  EnsureServer();
  m_server->AddVariable(name, start_val);
  m_var_callbacks[name] = cb;
}

sup::dto::AnyValue PvAccessSharedServer::GetValue(const std::string& name)
{
  return m_server->GetValue(name);
}

bool PvAccessSharedServer::SetValue(const std::string& name, const sup::dto::AnyValue& value)
{
  return m_server->SetValue(name, value);
}

void PvAccessSharedServer::Setup()
{
  EnsureServer();
  m_server->Start();
}

void PvAccessSharedServer::Teardown()
{
  m_server.reset();
}

void PvAccessSharedServer::EnsureServer()
{
  if (!m_server)
  {
    auto callback = [this](const std::string& name, const sup::dto::AnyValue& value) {
      DelegateCallbacks(name, value);
    };
    m_server.reset(new epics::PvAccessServer(callback));
  }
}

void PvAccessSharedServer::DelegateCallbacks(const std::string& name,
                                             const sup::dto::AnyValue& value)
{
  auto iter = m_var_callbacks.find(name);
  if (iter == m_var_callbacks.end())
  {
    return;
  }
  iter->second(value);
}

}  // namespace sequencer

}  // namespace sup
