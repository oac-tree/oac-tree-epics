/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Walter Van Herck (IO)
 *
 * Copyright (c) : 2010-2026 ITER Organization,
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

#include "pv_access_encoded_server_variable.h"

#include "pv_access_server_variable.h"
#include "pv_access_helper.h"
#include "pv_access_shared_server_registry.h"

#include <sup/oac-tree/concrete_constraints.h>
#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/variable_registry.h>
#include <sup/oac-tree/workspace.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>
#include <sup/dto/json_value_parser.h>
#include <sup/protocol/base64_variable_codec.h>

namespace sup
{
namespace oac_tree
{
const std::string PvAccessEncodedServerVariable::Type = "PvAccessEncodedServer";
static bool PvAccessEncodedServerVariable_initialised_flag =
  RegisterGlobalVariable<PvAccessEncodedServerVariable>();

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string TYPE_ATTRIBUTE_NAME = "type";
const std::string VALUE_ATTRIBUTE_NAME = "value";

PvAccessEncodedServerVariable::PvAccessEncodedServerVariable()
  : Variable(PvAccessEncodedServerVariable::Type)
  , m_initial_type{}
  , m_workspace{nullptr}
{
  (void)AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
  (void)AddAttributeDefinition(TYPE_ATTRIBUTE_NAME, sup::dto::StringType);
  (void)AddAttributeDefinition(VALUE_ATTRIBUTE_NAME, sup::dto::StringType);
  AddConstraint(MakeConstraint<Or>(
    MakeConstraint<Exists>(TYPE_ATTRIBUTE_NAME),
    MakeConstraint<Not>(MakeConstraint<Exists>(VALUE_ATTRIBUTE_NAME))));
}

PvAccessEncodedServerVariable::~PvAccessEncodedServerVariable() = default;

PvAccessSharedServer& PvAccessEncodedServerVariable::GetSharedServer() const
{
  if (m_workspace == nullptr)
  {
    throw InvalidOperationException(
      "PvAccessEncodedServerVariable::GetSharedServer(): Workspace is not set");
  }
  return pv_access_helper::GetSharedPvAccessServerRegistry().GetServer(m_workspace);
}

bool PvAccessEncodedServerVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  auto encoded = GetSharedServer().GetValue(GetAttributeString(CHANNEL_ATTRIBUTE_NAME));
  auto decoded = sup::protocol::Base64VariableCodec::Decode(encoded);
  return decoded.first && sup::dto::TryAssign(value, decoded.second);
}

bool PvAccessEncodedServerVariable::SetValueImpl(const sup::dto::AnyValue& value)
{
  auto encoded = sup::protocol::Base64VariableCodec::Encode(value);
  return GetSharedServer().SetValue(GetAttributeString(CHANNEL_ATTRIBUTE_NAME), encoded.second);
}

bool PvAccessEncodedServerVariable::IsAvailableImpl() const
{
  auto value = GetSharedServer().GetValue(GetAttributeString(CHANNEL_ATTRIBUTE_NAME));
  auto decoded = sup::protocol::Base64VariableCodec::Decode(value);
  return decoded.first;
}

SetupTeardownActions PvAccessEncodedServerVariable::SetupImpl(const Workspace& ws)
{
  m_workspace = std::addressof(ws);
  if (HasAttribute(TYPE_ATTRIBUTE_NAME))
  {
    auto type_attr_val = GetAttributeString(TYPE_ATTRIBUTE_NAME);
    const auto& registry = ws.GetTypeRegistry();
    sup::dto::JSONAnyTypeParser parser;
    if (!parser.ParseString(type_attr_val, std::addressof(registry)))
    {
      std::string error_message = VariableSetupExceptionProlog(*this) +
        "could not parse attribute [" + TYPE_ATTRIBUTE_NAME + "] with value [" + type_attr_val + "]";
      throw VariableSetupException(error_message);
    }
    m_initial_type = parser.MoveAnyType();
  }
  auto val = GetInitialValue(*this, m_initial_type);
  auto callback = [this](const sup::dto::AnyValue& value)
  {
    auto decoded = sup::protocol::Base64VariableCodec::Decode(value);
    // Notify with empty value if decoding failed
    Notify(decoded.second, true);
    return;
  };
  auto encoded = sup::protocol::Base64VariableCodec::Encode(val);
  GetSharedServer().AddVariable(GetAttributeString(CHANNEL_ATTRIBUTE_NAME), encoded.second,
                                callback);
  // Use same key as standard PvAccess server variable:
  SetupTeardownActions actions{
    PvAccessServerVariable::Type,
    [workspace = m_workspace]() {
      pv_access_helper::GetSharedPvAccessServerRegistry().Setup(workspace);
    },
    [workspace = m_workspace]() {
      pv_access_helper::GetSharedPvAccessServerRegistry().Teardown(workspace);
    }
  };
  Notify(val, true);
  return actions;
}

void PvAccessEncodedServerVariable::ResetImpl(const Workspace& ws)
{
  (void)ws;
  auto val = GetInitialValue(*this, m_initial_type);
  auto encoded = sup::protocol::Base64VariableCodec::Encode(val);
  (void)GetSharedServer().SetValue(GetAttributeString(CHANNEL_ATTRIBUTE_NAME), encoded.second);
}

void PvAccessEncodedServerVariable::TeardownImpl()
{
  m_initial_type = sup::dto::EmptyType;
  m_workspace = nullptr;
}

}  // namespace oac_tree

}  // namespace sup
