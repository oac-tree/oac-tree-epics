/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : Instruction node implementation
*
* Author        : Bertrand Bauvir (IO)
*
* Copyright (c) : 2010-2021 ITER Organization,
*                 CS 90 046
*                 13067 St. Paul-lez-Durance Cedex
*                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

#include "channel_access_client_variable.h"
#include "channel_access_helper.h"

#include <sup/sequencer/concrete_constraints.h>
#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>
#include <sup/epics/channel_access_pv.h>

namespace sup {

namespace sequencer {

const std::string ChannelAccessClientVariable::Type = "ChannelAccessClient";
static bool _cavariable_registered_flag = RegisterGlobalVariable<ChannelAccessClientVariable>();

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string TYPE_ATTRIBUTE_NAME = "type";

ChannelAccessClientVariable::ChannelAccessClientVariable()
  : Variable(ChannelAccessClientVariable::Type)
  , m_pv{}
  , m_type{}
{
  AddAttributeDefinition(CHANNEL_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
  AddAttributeDefinition(TYPE_ATTRIBUTE_NAME, sup::dto::StringType).SetMandatory();
}

ChannelAccessClientVariable::~ChannelAccessClientVariable() = default;

bool ChannelAccessClientVariable::GetValueImpl(sup::dto::AnyValue &value) const
{
  if (!m_pv)
  {
    return false;
  }
  if (!m_type.HasField(channel_access_helper::CONNECTED_FIELD_NAME) && !m_pv->IsConnected())
  {
    return false;
  }
  auto ext_value = m_pv->GetExtendedValue();
  auto result = channel_access_helper::ConvertToTypedAnyValue(ext_value, m_type);
  return sup::dto::TryAssign(value, result);
}

bool ChannelAccessClientVariable::SetValueImpl(const sup::dto::AnyValue &value)
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  if (sup::dto::IsStructValue(value))
  {
    if (!value.HasField(channel_access_helper::VALUE_FIELD_NAME))
    {
      return false;
    }
    return m_pv->SetValue(value[channel_access_helper::VALUE_FIELD_NAME]);
  }
  return m_pv->SetValue(value);
}

bool ChannelAccessClientVariable::IsAvailableImpl() const
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto ext_value = m_pv->GetExtendedValue();
  return !sup::dto::IsEmptyValue(ext_value.value);
}

SetupTeardownActions ChannelAccessClientVariable::SetupImpl(const Workspace& ws)
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
  auto channel_type = channel_access_helper::ChannelType(m_type);
  if (sup::dto::IsEmptyType(channel_type))
  {
    std::string error_message = VariableSetupExceptionProlog(*this) +
      "parsed channel type [" + type_attr_val + "] is not supported";
    throw VariableSetupException(error_message);
  }
  auto callback =
    [this](const epics::ChannelAccessPV::ExtendedValue& ext_value) {
      auto value = channel_access_helper::ConvertToTypedAnyValue(ext_value, m_type);
      Notify(value, ext_value.connected);
      return;
    };
  m_pv.reset(new epics::ChannelAccessPV(GetAttributeString(CHANNEL_ATTRIBUTE_NAME),
                                        channel_type, callback));
  return {};
}

void ChannelAccessClientVariable::TeardownImpl()
{
  m_pv.reset();
  m_type = sup::dto::EmptyType;
}

} // namespace sequencer

} // namespace sup
