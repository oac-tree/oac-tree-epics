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

#include "unit_test_helper.h"

#include <sup/oac-tree/generic_utils.h>
#include <sup/oac-tree/variable.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue_helper.h>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <thread>

namespace sup {

namespace oac_tree {

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
  return sup::dto::TryAssign(value, m_value);
}

bool ReadOnlyVariable::SetValueImpl(const sup::dto::AnyValue&)
{
  return false;
}

std::string CreateProcedureString(const std::string &body)
{
  static const std::string header{
      R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/oac-tree" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/oac-tree oac-tree.xsd">)RAW"};

  static const std::string footer{R"RAW(</Procedure>)RAW"};

  return header + body + footer;
}

} // namespace unit_test_helper

} // namespace oac_tree

} // namespace sup
