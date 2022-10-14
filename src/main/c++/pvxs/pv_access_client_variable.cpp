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

#include <sup/sequencer/variable_registry.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>
#include <sup/epics/pv_access_client_pv.h>
#include <sup/epics/pv_access_context_utils.h>

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
  , m_pv{}
  , m_type{}
{}

PvAccessClientVariable::~PvAccessClientVariable() = default;

bool PvAccessClientVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto cached_val = m_pv->GetValue();
  return sup::dto::TryConvert(value, cached_val);
}

bool PvAccessClientVariable::SetValueImpl(const sup::dto::AnyValue& value)
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  sup::dto::AnyValue copy(m_type);
  if (!sup::dto::TryConvert(copy, value))
  {
    return false;
  }
  return m_pv->SetValue(copy);
}

bool PvAccessClientVariable::IsAvailableImpl() const
{
  if (!m_pv || !m_pv->IsConnected())
  {
    return false;
  }
  auto value = m_pv->GetValue();
  return !sup::dto::IsEmptyValue(value);
}

bool PvAccessClientVariable::SetupImpl(const sup::dto::AnyTypeRegistry& registry)
{
  if (!HasAttribute(CHANNEL_ATTRIBUTE_NAME) || !HasAttribute(TYPE_ATTRIBUTE_NAME))
  {
    return false;
  }
  sup::dto::JSONAnyTypeParser parser;
  if (!parser.ParseString(GetAttribute(TYPE_ATTRIBUTE_NAME), &registry))
  {
    return false;
  }
  m_type = parser.MoveAnyType();
  auto callback = [this](const sup::dto::AnyValue& value)
                  {
                    Notify(value);
                    return;
                  };
  m_pv.reset(new epics::PvAccessClientPV(GetAttribute(CHANNEL_ATTRIBUTE_NAME),
                                         epics::CreateClientContextFromEnv(), callback));
  return true;
}

void PvAccessClientVariable::ResetImpl()
{
  m_pv.reset();
  m_type = sup::dto::EmptyType;
}

}  // namespace sequencer

}  // namespace sup
