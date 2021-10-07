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

  //! Inits variable type. Will throw is type can't be parsed.
  void InitType(const std::string& datatype)
  {
    if (ccs::HelperTools::Parse(m_var_type, datatype.c_str()) <= 0)
    {
      throw std::runtime_error("Error: can't parse datatype");
    }
  }

  //! Adds variable to the cache. Will throw if it is not possible.
  void AddVariable(const std::string& channel)
  {
    m_channel = channel;
    if (!m_client.AddVariable(m_channel.c_str(), ccs::types::AnyputVariable))
    {
      throw std::runtime_error("Error: can't add variable");
    }
  }

  // Launch the client.
  void Launch()
  {
    if (!m_client.Launch())
    {
      throw std::runtime_error("Error: can't launch the client");
    }
    // TODO Consider asking the server about existing variable already at this stage
    // throw if such variable doesn't exist or doesn't have a correct type.
  }
};

PVClientVariable::PVClientVariable()
    : Variable(PVClientVariable::Type), p_impl(new PVClientVariableImpl)
{
}

PVClientVariable::~PVClientVariable()
{
  delete p_impl;
}

bool PVClientVariable::SetupImpl()
{
  const bool has_attributes = HasAttribute("channel") && HasAttribute("datatype");
  if (!has_attributes)
  {
    return false;
  }

  p_impl->InitType(GetAttribute("datatype"));
  p_impl->AddVariable(GetAttribute("channel"));
  p_impl->Launch();

  return true;
}

bool PVClientVariable::GetValueImpl(ccs::types::AnyValue& value) const
{
  return false;
}

bool PVClientVariable::SetValueImpl(const ccs::types::AnyValue& value)
{
  return false;
}

}  // namespace sequencer

}  // namespace sup
