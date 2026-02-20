/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Walter Van Herck
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

#include "pv_access_encoded_client_variable.h"

#include <sup/oac-tree/variable_registry.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/epics/pv_access_client_pv.h>
#include <sup/protocol/base64_variable_codec.h>

namespace sup
{
namespace oac_tree
{

const std::string PvAccessEncodedClientVariable::Type = "PvAccessEncodedClient";
static bool PvAccessEncodedClientVariable_initialised_flag =
  RegisterGlobalVariable<PvAccessEncodedClientVariable>();

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";

PvAccessEncodedClientVariable::PvAccessEncodedClientVariable()
  : Variable(PvAccessEncodedClientVariable::Type)
  , m_pv{}
{
  (void)AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
}

PvAccessEncodedClientVariable::~PvAccessEncodedClientVariable() = default;

bool PvAccessEncodedClientVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto decoded = sup::protocol::Base64VariableCodec::Decode(m_pv->GetValue());
  return decoded.first && sup::dto::TryAssign(value, decoded.second);
}

bool PvAccessEncodedClientVariable::SetValueImpl(const sup::dto::AnyValue& value)
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto encoded = sup::protocol::Base64VariableCodec::Encode(value);
  return m_pv->SetValue(encoded.second);
}

bool PvAccessEncodedClientVariable::IsAvailableImpl() const
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto decoded = sup::protocol::Base64VariableCodec::Decode(m_pv->GetValue());
  return decoded.first;
}

SetupTeardownActions PvAccessEncodedClientVariable::SetupImpl(const Workspace& ws)
{
  (void)ws;
  auto callback = [this](const epics::PvAccessClientPV::ExtendedValue& ext_value)
  {
    auto decoded = sup::protocol::Base64VariableCodec::Decode(ext_value.value);
    // Notify with empty value if decoding failed
    Notify(decoded.second, ext_value.connected);
    return;
  };
  m_pv = std::make_unique<epics::PvAccessClientPV>(GetAttributeString(CHANNEL_ATTRIBUTE_NAME),
                                                   callback);
  return {};
}

void PvAccessEncodedClientVariable::TeardownImpl()
{
  m_pv.reset();
}

}  // namespace oac_tree

}  // namespace sup
