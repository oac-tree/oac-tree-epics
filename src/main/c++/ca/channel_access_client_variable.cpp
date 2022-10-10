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
{}

ChannelAccessClientVariable::~ChannelAccessClientVariable() = default;

bool ChannelAccessClientVariable::GetValueImpl(sup::dto::AnyValue &value) const
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto ext_value = m_pv->GetExtendedValue();
  // TODO: copy required fields into value of right type and set value
  return true;
}

bool ChannelAccessClientVariable::SetValueImpl(const sup::dto::AnyValue &value)
{
  // bool status = client.IsValid(channel.c_str());

  // if (status)
  // {
  //   status = client.SetAnyValue(channel.c_str(), value);
  // }
  // return status;
  return true;
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
                    Notify(ext_value.value);
                    return;
                  };
  m_pv.reset(new epics::ChannelAccessPV(GetAttribute(CHANNEL_ATTRIBUTE_NAME), m_type, callback));
  return true;
}

void ChannelAccessClientVariable::ResetImpl()
{
  m_pv.reset();
  m_type = sup::dto::EmptyType;
}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
