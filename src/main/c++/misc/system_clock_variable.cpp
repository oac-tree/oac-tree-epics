/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : System clock variable implementation
*
* Author        : Bertrand Bauvir (IO)
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

#include "system_clock_variable.h"

#include <sup/sequencer/generic_utils.h>
#include <sup/sequencer/variable_registry.h>

#include <sup/dto/anyvalue_helper.h>
#include <sup/dto/json_type_parser.h>

#include <ctime>

namespace
{
std::string GetISO8601Representation(unsigned long timestamp);
}  // unnamed namespace

namespace sup {

namespace sequencer {

const std::string SystemClockVariable::Type = "SystemClock";
static bool _sysclockvariable_initialised_flag = RegisterGlobalVariable<SystemClockVariable>();

const std::string DATATYPE_ATTRIBUTE_NAME = "datatype";

SystemClockVariable::SystemClockVariable()
  : Variable(SystemClockVariable::Type)
  , m_time_type{}
{
  // Register timespec equivalent type to the GlobalTypeDatabase
  // ::ccs::types::char8 type [] = "{\"type\":\"sup::FractionalTime/v1.0\",\"attributes\":["
  //   "{\"seconds\":{\"type\":\"uint32\"}},"
  //   "{\"nanosec\":{\"type\":\"uint32\"}}"
  //   "]}";
  // (void)::ccs::base::GlobalTypeDatabase::Register(type);
}

SystemClockVariable::~SystemClockVariable() = default;

bool SystemClockVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  unsigned long time = utils::GetNanosecsSinceEpoch();

  sup::dto::AnyValue result{m_time_type};

  if (m_time_type == sup::dto::UnsignedInteger64Type)
  {
    result = time;
  }
  else if (m_time_type == sup::dto::StringType)
  {
    result = GetISO8601Representation(time);
  }
  // else if (std::string(_type->GetName()) == "sup::FractionalTime/v1.0")
  // {
  //   struct { ::ccs::types::uint32 secs; ::ccs::types::uint32 nsec; } _fract = { 0u, 0u};
  //   _fract.secs = static_cast<::ccs::types::uint32>(_time / 1000000000ul);
  //   _fract.nsec = static_cast<::ccs::types::uint32>(_time - (1000000000ul * static_cast<::ccs::types::uint64>(_fract.secs)));
  //   result = _fract;
  // }
  else
  {
    return false;
  }
  return sup::dto::TryConvert(value, result);
}

bool SystemClockVariable::SetValueImpl(const sup::dto::AnyValue&)
{
  return false;
}

bool SystemClockVariable::SetupImpl(const sup::dto::AnyTypeRegistry& registry)
{
  if (HasAttribute(DATATYPE_ATTRIBUTE_NAME))
  {
    sup::dto::JSONAnyTypeParser parser;
    if (!parser.ParseString(GetAttribute(DATATYPE_ATTRIBUTE_NAME)))
    {
      return false;
    }
    m_time_type = parser.MoveAnyType();
  }
  else
  {
    m_time_type = sup::dto::UnsignedInteger64Type;
  }
  return true;
}

void SystemClockVariable::ResetImpl()
{
  m_time_type = sup::dto::EmptyType;
}

} // namespace sequencer

} // namespace sup

namespace
{
std::string GetISO8601Representation(unsigned long timestamp)
{
    // std::time_t seconds_after_epoch = timestamp / 1000000000;
    // char buf[sizeof("2011-10-08T07:07:09Z")];
    // std::strftime(buf, sizeof(buf), "%FT%TZ", std::gmtime(&seconds_after_epoch));
    return {};
}
}  // unnamed namespace

