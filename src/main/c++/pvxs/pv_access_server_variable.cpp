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

#include "pv_access_server_variable.h"

#include "pv_access_helper.h"

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/variable_registry.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>
#include <sup/dto/json_value_parser.h>
#include <sup/epics/pv_access_server.h>

namespace sup
{
namespace sequencer
{
const std::string PvAccessServerVariable::Type = "PvAccessServer";
static bool PvAccessServerVariable_initialised_flag =
  RegisterGlobalVariable<PvAccessServerVariable>();

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string TYPE_ATTRIBUTE_NAME = "type";
const std::string VALUE_ATTRIBUTE_NAME = "value";

PvAccessServerVariable::PvAccessServerVariable()
  : Variable(PvAccessServerVariable::Type)
  , m_type{}
  , m_server{}
{}

PvAccessServerVariable::~PvAccessServerVariable() = default;

bool PvAccessServerVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  if (!m_server || !m_type || !HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    return false;
  }
  auto converted_val = pv_access_helper::ConvertToTypedAnyValue(
    m_server->GetValue(GetAttribute(CHANNEL_ATTRIBUTE_NAME)), *m_type);
  return !sup::dto::IsEmptyValue(converted_val) && sup::dto::TryConvert(value, converted_val);
}

bool PvAccessServerVariable::SetValueImpl(const sup::dto::AnyValue& value)
{
  if (!m_server || !m_type || !HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    return false;
  }
  sup::dto::AnyValue copy(*m_type);
  if (!sup::dto::TryConvert(copy, value))
  {
    return false;
  }
  return m_server->SetValue(GetAttribute(CHANNEL_ATTRIBUTE_NAME),
                            pv_access_helper::PackIntoStructIfScalar(copy));
}

bool PvAccessServerVariable::IsAvailableImpl() const
{
  if (!m_server || !m_type || !HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    return false;
  }
  auto value = pv_access_helper::ConvertToTypedAnyValue(
    m_server->GetValue(GetAttribute(CHANNEL_ATTRIBUTE_NAME)), *m_type);
  return !sup::dto::IsEmptyValue(value);
}

void PvAccessServerVariable::SetupImpl(const sup::dto::AnyTypeRegistry& registry)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME))
  {
    std::string error_message = VariableSetupExceptionProlog(GetName(), Type) +
      "missing mandatory attribute [" + CHANNEL_ATTRIBUTE_NAME + "]";
    throw VariableSetupException(error_message);
  }
  if (!HasAttribute(TYPE_ATTRIBUTE_NAME))
  {
    std::string error_message = VariableSetupExceptionProlog(GetName(), Type) +
      "missing mandatory attribute [" + TYPE_ATTRIBUTE_NAME + "]";
    throw VariableSetupException(error_message);
  }
  sup::dto::JSONAnyTypeParser parser;
  auto type_attr_val = GetAttribute(TYPE_ATTRIBUTE_NAME);
  if (!parser.ParseString(type_attr_val, &registry))
  {
    std::string error_message = VariableSetupExceptionProlog(GetName(), Type) +
      "could not parse attribute [" + TYPE_ATTRIBUTE_NAME + "] with value [" + type_attr_val + "]";
    throw VariableSetupException(error_message);
  }
  m_type.reset(new sup::dto::AnyType(parser.MoveAnyType()));
  sup::dto::AnyValue val(*m_type);
  if (HasAttribute(VALUE_ATTRIBUTE_NAME))
  {
    auto val_str = GetAttribute(VALUE_ATTRIBUTE_NAME);
    sup::dto::JSONAnyValueParser value_parser;
    if (!value_parser.TypedParseString(*m_type, val_str))
    {
      std::string error_message = VariableSetupExceptionProlog(GetName(), Type) +
        "could not parse attribute [" + VALUE_ATTRIBUTE_NAME + "] with value [" + val_str + "]";
      throw VariableSetupException(error_message);
    }
    val = value_parser.MoveAnyValue();
  }
  // Avoid dependence on destruction order of m_server and m_type.
  sup::dto::AnyType type_copy{*m_type};
  auto callback = [this, type_copy](const std::string&, const sup::dto::AnyValue& value)
  {
    auto typed_value = pv_access_helper::ConvertToTypedAnyValue(value, type_copy);
    Notify(typed_value);
    return;
  };
  m_server.reset(new epics::PvAccessServer(callback));
  auto start_value = pv_access_helper::PackIntoStructIfScalar(val);
  m_server->AddVariable(GetAttribute(CHANNEL_ATTRIBUTE_NAME), start_value);
  m_server->Start();
}

void PvAccessServerVariable::ResetImpl()
{
  m_server.reset();
  m_type.reset();
}

}  // namespace sequencer

}  // namespace sup
