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

#include "unit_test_helper.h"

#include <sup/sequencer/generic_utils.h>
#include <sup/sequencer/variable.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue_helper.h>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <thread>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

bool WaitForCAChannel(const std::string& channel, const std::string& type_str, double timeout)
{
  Workspace ws;
  auto variable = GlobalVariableRegistry().Create("ChannelAccessClient");
  if (!variable)
  {
    return false;
  }
  variable->AddAttribute("channel", channel);
  variable->AddAttribute("type", type_str);
  ws.AddVariable("var", std::move(variable));
  ws.Setup();

  return ws.WaitForVariable("var", timeout);
}

ReadOnlyVariable::ReadOnlyVariable(const sup::dto::AnyValue& value)
  : Variable("UnitTest_ReadOnlyVariable")
  , m_value{value}
{}

ReadOnlyVariable::~ReadOnlyVariable() = default;

bool ReadOnlyVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  return sup::dto::TryConvert(value, m_value);
}

bool ReadOnlyVariable::SetValueImpl(const sup::dto::AnyValue&)
{
  return false;
}

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup
