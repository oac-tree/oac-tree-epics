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

#include <sup/sequencer/variable_registry.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>
#include <sup/epics/channel_access_pv.h>

namespace
{
sup::dto::AnyType ChannelType(const sup::dto::AnyType& anytype);

sup::dto::AnyValue ConvertToTypedAnyValue(
  const sup::epics::ChannelAccessPV::ExtendedValue& ext_value, const sup::dto::AnyType& anytype);

const std::string VALUE_FIELD_NAME = "value";
const std::string CONNECTED_FIELD_NAME = "connected";
const std::string TIMESTAMP_FIELD_NAME = "timestamp";
const std::string STATUS_FIELD_NAME = "status";
const std::string SEVERITY_FIELD_NAME = "severity";

}  // unnamed namespace

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
{}

ChannelAccessClientVariable::~ChannelAccessClientVariable() = default;

bool ChannelAccessClientVariable::WaitForConnected(double timeout_sec) const
{
  if (!m_pv)
  {
    return false;
  }
  return m_pv->WaitForConnected(timeout_sec);
}

bool ChannelAccessClientVariable::GetValueImpl(sup::dto::AnyValue &value) const
{
  if (!m_pv)
  {
    return false;
  }
  if (!m_type.HasField(CONNECTED_FIELD_NAME) && !m_pv->IsConnected())
  {
    return false;
  }
  auto ext_value = m_pv->GetExtendedValue();
  auto result = ConvertToTypedAnyValue(ext_value, m_type);
  return sup::dto::TryConvert(value, result);
}

bool ChannelAccessClientVariable::SetValueImpl(const sup::dto::AnyValue &value)
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  if (sup::dto::IsStructValue(value))
  {
    if (!value.HasField(VALUE_FIELD_NAME))
    {
      return false;
    }
    return m_pv->SetValue(value[VALUE_FIELD_NAME]);
  }
  return m_pv->SetValue(value);
}

bool ChannelAccessClientVariable::SetupImpl(const sup::dto::AnyTypeRegistry& registry)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME) || !HasAttribute(TYPE_ATTRIBUTE_NAME))
  {
    return false;
  }
  sup::dto::JSONAnyTypeParser parser;
  if (!parser.ParseString(GetAttribute(TYPE_ATTRIBUTE_NAME)))
  {
    return false;
  }
  m_type = parser.MoveAnyType();
  auto callback = [this](const epics::ChannelAccessPV::ExtendedValue& ext_value)
                  {
                    auto value = ConvertToTypedAnyValue(ext_value, m_type);
                    Notify(value);
                    return;
                  };
  m_pv.reset(new epics::ChannelAccessPV(GetAttribute(CHANNEL_ATTRIBUTE_NAME),
                                        ChannelType(m_type), callback));
  return true;
}

void ChannelAccessClientVariable::ResetImpl()
{
  m_pv.reset();
  m_type = sup::dto::EmptyType;
}

} // namespace sequencer

} // namespace sup

namespace
{
sup::dto::AnyType ChannelType(const sup::dto::AnyType& anytype)
{
  if (!sup::dto::IsStructType(anytype))
  {
    return anytype;
  }
  if (anytype.HasField(VALUE_FIELD_NAME))
  {
    return anytype[VALUE_FIELD_NAME];
  }
  return {};
}

sup::dto::AnyValue ConvertToTypedAnyValue(
  const sup::epics::ChannelAccessPV::ExtendedValue& ext_value, const sup::dto::AnyType& anytype)
{
  if (ext_value.value.GetType() == anytype)
  {
    return ext_value.value;
  }
  sup::dto::AnyValue result(anytype);
  if (!anytype.HasField(VALUE_FIELD_NAME) ||
      !sup::dto::TryConvert(result[VALUE_FIELD_NAME], ext_value.value))
  {
    return {};
  }
  if (anytype.HasField(CONNECTED_FIELD_NAME) &&
      !sup::dto::TryConvert(result[CONNECTED_FIELD_NAME], ext_value.connected))
  {
    return {};
  }
  if (anytype.HasField(TIMESTAMP_FIELD_NAME) &&
      !sup::dto::TryConvert(result[TIMESTAMP_FIELD_NAME], ext_value.timestamp))
  {
    return {};
  }
  if (anytype.HasField(STATUS_FIELD_NAME) &&
      !sup::dto::TryConvert(result[STATUS_FIELD_NAME], ext_value.status))
  {
    return {};
  }
  if (anytype.HasField(SEVERITY_FIELD_NAME) &&
      !sup::dto::TryConvert(result[SEVERITY_FIELD_NAME], ext_value.severity))
  {
    return {};
  }
  return result;
}
}  // unnamed namespace
