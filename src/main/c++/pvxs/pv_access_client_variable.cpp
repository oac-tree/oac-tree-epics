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

#include "pv_access_client_variable.h"

#include "pv_access_helper.h"

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/variable_registry.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>
#include <sup/epics/pv_access_client_pv.h>

namespace sup
{
namespace sequencer
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
{}

PvAccessClientVariable::~PvAccessClientVariable() = default;

bool PvAccessClientVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto converted_val = m_type ? pv_access_helper::ConvertToTypedAnyValue(m_pv->GetValue(), *m_type)
                              : m_pv->GetValue();
  return !sup::dto::IsEmptyValue(converted_val) && sup::dto::TryConvert(value, converted_val);
}

bool PvAccessClientVariable::SetValueImpl(const sup::dto::AnyValue& value)
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  sup::dto::AnyValue copy;
  if (!m_type)
  {
    copy = value;
  }
  else
  {
    copy = sup::dto::AnyValue(*m_type);
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
  auto value = m_type ? pv_access_helper::ConvertToTypedAnyValue(m_pv->GetValue(), *m_type)
                      : m_pv->GetValue();
  return !sup::dto::IsEmptyValue(value);
}

void PvAccessClientVariable::SetupImpl(const sup::dto::AnyTypeRegistry& registry)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    std::string error_message = VariableSetupExceptionProlog(GetName(), Type) +
      "missing mandatory attribute [" + CHANNEL_ATTRIBUTE_NAME + "]";
    throw VariableSetupException(error_message);
  }
  if (HasAttribute(TYPE_ATTRIBUTE_NAME))
  {
    // TODO: temporary hack for GUI! Remove this!
    auto type_attr = GetAttribute(TYPE_ATTRIBUTE_NAME);
    if (type_attr.empty())
    {
      std::string error_message = VariableSetupExceptionProlog(GetName(), Type) +
        "type attribute [" + TYPE_ATTRIBUTE_NAME + "] is empty";
      throw VariableSetupException(error_message);
    }
    sup::dto::JSONAnyTypeParser parser;
    auto type_str = GetAttribute(TYPE_ATTRIBUTE_NAME);
    if (!parser.ParseString(type_str, &registry))
    {
      std::string error_message = VariableSetupExceptionProlog(GetName(), Type) +
        "could not parse type [" + type_str + "]";
      throw VariableSetupException(error_message);
    }
    m_type.reset(new sup::dto::AnyType(parser.MoveAnyType()));
  }
  // Avoid dependence on destruction order of m_pv and m_type.
  sup::dto::AnyType type_copy = m_type ? *m_type : sup::dto::EmptyType;
  auto callback = [this, type_copy](const epics::PvAccessClientPV::ExtendedValue& ext_value)
  {
    auto value = sup::dto::IsEmptyType(type_copy) ? ext_value.value
                    : pv_access_helper::ConvertToTypedAnyValue(ext_value.value, type_copy);
    Notify(value);
    return;
  };
  m_pv.reset(new epics::PvAccessClientPV(GetAttribute(CHANNEL_ATTRIBUTE_NAME), callback));
}

void PvAccessClientVariable::ResetImpl()
{
  m_pv.reset();
  m_type.reset();
}

}  // namespace sequencer

}  // namespace sup
