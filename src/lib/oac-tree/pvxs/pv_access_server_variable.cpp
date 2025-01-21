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
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#include "pv_access_server_variable.h"

#include "pv_access_helper.h"
#include "pv_access_shared_server.h"

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>
#include <sup/dto/json_value_parser.h>

namespace sup
{
namespace oac_tree
{
const std::string PvAccessServerVariable::Type = "PvAccessServer";
static bool PvAccessServerVariable_initialised_flag =
  RegisterGlobalVariable<PvAccessServerVariable>();

PvAccessSharedServer& GetSharedPvAccessServer()
{
  static PvAccessSharedServer shared_server{};
  return shared_server;
}

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string TYPE_ATTRIBUTE_NAME = "type";
const std::string VALUE_ATTRIBUTE_NAME = "value";

PvAccessServerVariable::PvAccessServerVariable()
  : Variable(PvAccessServerVariable::Type)
  , m_type{}
{
  AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
  AddAttributeDefinition(TYPE_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
  AddAttributeDefinition(VALUE_ATTRIBUTE_NAME, sup::dto::StringType);
}

PvAccessServerVariable::~PvAccessServerVariable() = default;

sup::dto::AnyValue PvAccessServerVariable::GetInitialValue(const sup::dto::AnyType& val_type) const
{
  sup::dto::AnyValue val(val_type);
  if (HasAttribute(VALUE_ATTRIBUTE_NAME))
  {
    auto val_str = GetAttributeString(VALUE_ATTRIBUTE_NAME);
    sup::dto::JSONAnyValueParser value_parser;
    if (!value_parser.TypedParseString(m_type, val_str))
    {
      std::string error_message = VariableSetupExceptionProlog(*this) +
        "could not parse attribute [" + VALUE_ATTRIBUTE_NAME + "] with value [" + val_str + "]";
      throw VariableSetupException(error_message);
    }
    val = value_parser.MoveAnyValue();
  }
  return val;
}

bool PvAccessServerVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  auto converted_val = pv_access_helper::ConvertToTypedAnyValue(
    GetSharedPvAccessServer().GetValue(GetAttributeString(CHANNEL_ATTRIBUTE_NAME)), m_type);
  return !sup::dto::IsEmptyValue(converted_val) && sup::dto::TryAssign(value, converted_val);
}

bool PvAccessServerVariable::SetValueImpl(const sup::dto::AnyValue& value)
{
  sup::dto::AnyValue copy(m_type);
  if (!sup::dto::TryConvert(copy, value))
  {
    return false;
  }
  return GetSharedPvAccessServer().SetValue(GetAttributeString(CHANNEL_ATTRIBUTE_NAME),
                                            pv_access_helper::PackIntoStructIfScalar(copy));
}

bool PvAccessServerVariable::IsAvailableImpl() const
{
  auto value = pv_access_helper::ConvertToTypedAnyValue(
    GetSharedPvAccessServer().GetValue(GetAttributeString(CHANNEL_ATTRIBUTE_NAME)), m_type);
  return !sup::dto::IsEmptyValue(value);
}

SetupTeardownActions PvAccessServerVariable::SetupImpl(const Workspace& ws)
{
  sup::dto::JSONAnyTypeParser parser;
  auto type_attr_val = GetAttributeString(TYPE_ATTRIBUTE_NAME);
  const auto& registry = ws.GetTypeRegistry();
  if (!parser.ParseString(type_attr_val, std::addressof(registry)))
  {
    std::string error_message = VariableSetupExceptionProlog(*this) +
      "could not parse attribute [" + TYPE_ATTRIBUTE_NAME + "] with value [" + type_attr_val + "]";
    throw VariableSetupException(error_message);
  }
  m_type = parser.MoveAnyType();
  auto val = GetInitialValue(m_type);
  // Avoid dependence on destruction order of m_server and m_type.
  auto callback = [this](const sup::dto::AnyValue& value)
  {
    auto typed_value = pv_access_helper::ConvertToTypedAnyValue(value, m_type);
    Notify(typed_value, true);
    return;
  };
  auto start_value = pv_access_helper::PackIntoStructIfScalar(val);
  GetSharedPvAccessServer().AddVariable(GetAttributeString(CHANNEL_ATTRIBUTE_NAME), start_value,
                                        callback);
  SetupTeardownActions actions{
    PvAccessServerVariable::Type,
    []() { GetSharedPvAccessServer().Setup(); },
    []() { GetSharedPvAccessServer().Teardown(); }
  };
  Notify(val, true);
  return actions;
}

void PvAccessServerVariable::ResetImpl(const Workspace& ws)
{
  (void)ws;
  if (sup::dto::IsEmptyType(m_type))
  {
    return;
  }
  auto val = GetInitialValue(m_type);
  GetSharedPvAccessServer().SetValue(GetAttributeString(CHANNEL_ATTRIBUTE_NAME),
                                     pv_access_helper::PackIntoStructIfScalar(val));
}

void PvAccessServerVariable::TeardownImpl()
{
  m_type = sup::dto::EmptyType;
}

}  // namespace oac_tree

}  // namespace sup
