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

#include "PVClientVariable.h"

#include <VariableRegistry.h>
#include <common/PVAccessClient.h>

#include <stdexcept>

namespace sup
{
namespace sequencer
{
const std::string PVClientVariable::Type = "PVClientVariable";
static bool PVClientVariable_initialised_flag = RegisterGlobalVariable<PVClientVariable>();

//! Pointer implementation for PVClientVariable
struct PVClientVariable::PVClientVariableImpl
{
  ccs::base::PVAccessClient m_client;
  ccs::base::SharedReference<ccs::types::AnyType> m_var_type;
  std::string m_channel;

  //! Inits variable type.
  bool InitType(const std::string& datatype)
  {
    return ccs::HelperTools::Parse(m_var_type, datatype.c_str()) > 0;
  }

  //! Adds variable to the cache.
  bool AddVariable(const std::string& channel)
  {
    m_channel = channel;
    return m_client.AddVariable(m_channel.c_str(), ccs::types::AnyputVariable);
  }

  //! Sets callback for variable updates.
  bool SetCallback(const std::string& channel,
                   const std::function<void(const ccs::types::AnyValue&)>& cb)
  {
    return m_client.SetCallback(m_channel.c_str(), cb);
  }

  // Launch the client.
  bool Launch()
  {
    return m_client.Launch();
  }

  bool CheckConnection()
  {
    if (!m_client.IsConnected(m_channel.c_str()))
    {
      return false;
    }
    return true;
  }
};

PVClientVariable::PVClientVariable()
    : Variable(PVClientVariable::Type), p_impl(new PVClientVariableImpl)
{
}

PVClientVariable::~PVClientVariable()
{}

bool PVClientVariable::GetValueImpl(ccs::types::AnyValue& value) const
{
  if (!p_impl->CheckConnection() || !p_impl->m_client.GetVariable(p_impl->m_channel.c_str(), value))
  {
    return false;
  }
  return true;
}

bool PVClientVariable::SetValueImpl(const ccs::types::AnyValue& value)
{
  return p_impl->m_client.SetVariable(p_impl->m_channel.c_str(), value);
}

bool PVClientVariable::SetupImpl()
{
  const bool has_attributes = HasAttribute("channel") && HasAttribute("datatype");
  if (!has_attributes)
  {
    return false;
  }
  bool status = p_impl->InitType(GetAttribute("datatype"));
  if (status)
  {
    status = p_impl->AddVariable(GetAttribute("channel"));
  }
  if (status)
  {
    status = p_impl->SetCallback(GetAttribute("channel"),
                      [this](const ccs::types::AnyValue& value)
                      {
                        Notify(value);
                        return;
                      });
  }
  if (status)
  {
    status = p_impl->Launch();
  }
  return status;
}

void PVClientVariable::ResetImpl()
{
  p_impl.reset(new PVClientVariableImpl);
}

}  // namespace sequencer

}  // namespace sup
