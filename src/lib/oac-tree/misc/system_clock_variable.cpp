/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) oac-tree component
*
* Description   : System clock variable implementation
*
* Author        : Bertrand Bauvir (IO)
*
* Copyright (c) : 2010-2025 ITER Organization,
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

#include "system_clock_variable.h"

#include <sup/oac-tree/exceptions.h>
#include <sup/oac-tree/generic_utils.h>
#include <sup/oac-tree/variable_registry.h>

#include <sup/dto/anyvalue_helper.h>

#include <cstdio>
#include <ctime>
#include <set>

namespace
{
bool IsSupportedTimeFormat(const std::string& format);
sup::dto::AnyValue GetFormattedTime(unsigned long timestamp, const std::string& format);
std::string GetISO8601Representation(unsigned long timestamp);
}  // unnamed namespace

namespace sup {

namespace oac_tree {

const std::string SystemClockVariable::Type = "SystemClock";
static bool _sysclockvariable_initialised_flag = RegisterGlobalVariable<SystemClockVariable>();

const std::string TIMEFORMAT_ATTRIBUTE_NAME = "format";

const std::string DEFAULT_TIME_FORMAT = "uint64";
const std::string ISO8601_TIME_FORMAT = "ISO8601";

SystemClockVariable::SystemClockVariable()
  : Variable(SystemClockVariable::Type)
  , m_time_format{}
{
  AddAttributeDefinition(TIMEFORMAT_ATTRIBUTE_NAME, sup::dto::StringType);
}

SystemClockVariable::~SystemClockVariable() = default;

bool SystemClockVariable::GetValueImpl(sup::dto::AnyValue& value) const
{
  auto time_av = ReadCurrentTime();
  if (!time_av)
  {
    return false;
  }
  Notify(*time_av, true);
  return sup::dto::TryAssign(value, *time_av);
}

bool SystemClockVariable::SetValueImpl(const sup::dto::AnyValue&)
{
  return false;
}

SetupTeardownActions SystemClockVariable::SetupImpl(const Workspace&)
{
  if (HasAttribute(TIMEFORMAT_ATTRIBUTE_NAME))
  {
    auto time_format = GetAttributeString(TIMEFORMAT_ATTRIBUTE_NAME);
    if (!IsSupportedTimeFormat(time_format))
    {
      std::string error_message = VariableSetupExceptionProlog(*this) +
        "attribute [" + TIMEFORMAT_ATTRIBUTE_NAME + "] contains unsupported time format [" +
        time_format + "]";
      throw VariableSetupException(error_message);
    }
    m_time_format = time_format;
  }
  else
  {
    m_time_format = DEFAULT_TIME_FORMAT;
  }
  auto time_av = ReadCurrentTime();
  if (!time_av)
  {
    Notify({}, false);
  }
  else
  {
    Notify(*time_av, true);
  }
  return {};
}

void SystemClockVariable::TeardownImpl()
{
  m_time_format.clear();
}

std::unique_ptr<sup::dto::AnyValue> SystemClockVariable::ReadCurrentTime() const
{
  std::unique_ptr<sup::dto::AnyValue> result;
  unsigned long timestamp = utils::GetNanosecsSinceEpoch();
  sup::dto::AnyValue time_av = GetFormattedTime(timestamp, m_time_format);
  if (!sup::dto::IsEmptyValue(time_av))
  {
    result = std::make_unique<sup::dto::AnyValue>(std::move(time_av));
  }
  return result;
}

} // namespace oac_tree

} // namespace sup

namespace
{
bool IsSupportedTimeFormat(const std::string& format)
{
  static std::set<std::string> supported_formats{
    sup::oac_tree::DEFAULT_TIME_FORMAT,
    sup::oac_tree::ISO8601_TIME_FORMAT
  };
  auto it = supported_formats.find(format);
  return it != supported_formats.end();
}

sup::dto::AnyValue GetFormattedTime(unsigned long timestamp, const std::string& format)
{
  if (format == sup::oac_tree::DEFAULT_TIME_FORMAT)
  {
    return sup::dto::AnyValue{sup::dto::UnsignedInteger64Type, timestamp};
  }
  else if (format == sup::oac_tree::ISO8601_TIME_FORMAT)
  {
    return sup::dto::AnyValue{sup::dto::StringType, GetISO8601Representation(timestamp)};
  }
  return {};
}

std::string GetISO8601Representation(unsigned long timestamp)
{
  const std::size_t buffer_size = 31u;
  const int nanosec_offset = 19;
  char buf[buffer_size];
  std::time_t seconds_after_epoch = timestamp / 1000000000ul;
  unsigned long nanosecs = timestamp % 1000000000ul;
  std::strftime(buf, buffer_size, "%FT%T", std::gmtime(&seconds_after_epoch));
  std::snprintf(buf + nanosec_offset, buffer_size - nanosec_offset, ".%.9luZ", nanosecs);
  return std::string(buf);
}
}  // unnamed namespace
