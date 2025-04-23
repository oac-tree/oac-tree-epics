/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Gennady Pospelov
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

#include "pv_access_client_variable.h"

#include "pv_access_helper.h"

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>
#include <sup/epics/pv_access_client_pv.h>

namespace sup
{
namespace oac_tree
{
const std::string PvAccessClientVariable::Type = "PvAccessClient";
static bool PvAccessClientVariable_initialised_flag =
  RegisterGlobalVariable<PvAccessClientVariable>();

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string TYPE_ATTRIBUTE_NAME = "type";

PvAccessClientVariable::PvAccessClientVariable()
  : Variable(PvAccessClientVariable::Type)
  , m_type{}
  , m_pv{}
{
  AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
  AddAttributeDefinition(TYPE_ATTRIBUTE_NAME, sup::dto::StringType);
}

PvAccessClientVariable::~PvAccessClientVariable() = default;

bool PvAccessClientVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto converted_val = sup::dto::IsEmptyType(m_type)
                           ? m_pv->GetValue()
                           : pv_access_helper::ConvertToTypedAnyValue(m_pv->GetValue(), m_type);
  return !sup::dto::IsEmptyValue(converted_val) && sup::dto::TryAssign(value, converted_val);
}

bool PvAccessClientVariable::SetValueImpl(const sup::dto::AnyValue& value)
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  sup::dto::AnyValue copy;
  if (sup::dto::IsEmptyType(m_type))
  {
    copy = value;
  }
  else
  {
    copy = sup::dto::AnyValue(m_type);
    if (!sup::dto::TryConvert(copy, value))
    {
      return false;
    }
  }
  return m_pv->SetValue(pv_access_helper::PackIntoStructIfScalar(copy));
}

bool PvAccessClientVariable::IsAvailableImpl() const
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto value = sup::dto::IsEmptyType(m_type)
                   ? m_pv->GetValue()
                   : pv_access_helper::ConvertToTypedAnyValue(m_pv->GetValue(), m_type);
  return !sup::dto::IsEmptyValue(value);
}

SetupTeardownActions PvAccessClientVariable::SetupImpl(const Workspace& ws)
{
  if (HasAttribute(TYPE_ATTRIBUTE_NAME))
  {
    auto type_attr = GetAttributeString(TYPE_ATTRIBUTE_NAME);
    if (type_attr.empty())
    {
      std::string error_message = VariableSetupExceptionProlog(*this) +
        "type attribute [" + TYPE_ATTRIBUTE_NAME + "] is empty";
      throw VariableSetupException(error_message);
    }
    sup::dto::JSONAnyTypeParser parser;
    const auto& registry = ws.GetTypeRegistry();
    if (!parser.ParseString(type_attr, std::addressof(registry)))
    {
      std::string error_message = VariableSetupExceptionProlog(*this) +
        "could not parse type [" + type_attr + "]";
      throw VariableSetupException(error_message);
    }
    m_type = parser.MoveAnyType();
  }
  // Avoid dependence on destruction order of m_pv and m_type.
  auto callback = [this](const epics::PvAccessClientPV::ExtendedValue& ext_value)
  {
    auto value = sup::dto::IsEmptyType(m_type)
                     ? ext_value.value
                     : pv_access_helper::ConvertToTypedAnyValue(ext_value.value, m_type);
    Notify(value, ext_value.connected);
    return;
  };
  m_pv = std::make_unique<epics::PvAccessClientPV>(
    GetAttributeString(CHANNEL_ATTRIBUTE_NAME), callback);
  return {};
}

void PvAccessClientVariable::TeardownImpl()
{
  m_pv.reset();
  m_type = sup::dto::EmptyType;
}

}  // namespace oac_tree

}  // namespace sup
