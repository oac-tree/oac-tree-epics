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

#include "channel_access_helper.h"

#include <sup/dto/anyvalue_helper.h>

namespace
{
bool PopulateExtraFields(sup::dto::AnyValue& anyvalue,
                         const sup::epics::ChannelAccessPV::ExtendedValue& ext_value);
}  // unnamed namespace

namespace sup
{
namespace sequencer
{
namespace channel_access_helper
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

sup::dto::AnyValue ExtractChannelValue(const sup::dto::AnyValue& value)
{
  if (!sup::dto::IsStructValue(value))
  {
    return value;
  }
  if (value.HasField(VALUE_FIELD_NAME))
  {
    return value[VALUE_FIELD_NAME];
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
  if (!ext_value.connected)
  {
    if (!PopulateExtraFields(result, ext_value))
    {
      return {};
    }
    return result;
  }
  if (!anytype.HasField(VALUE_FIELD_NAME) ||
      !sup::dto::TryConvert(result[VALUE_FIELD_NAME], ext_value.value))
  {
    return {};
  }
  if (!PopulateExtraFields(result, ext_value))
  {
    return {};
  }
  return result;
}

} // namespace channel_access_helper

} // namespace sequencer

} // namespace sup

using namespace sup::sequencer::channel_access_helper;

namespace
{
bool PopulateExtraFields(sup::dto::AnyValue& anyvalue,
                         const sup::epics::ChannelAccessPV::ExtendedValue& ext_value)
{
  if (anyvalue.HasField(CONNECTED_FIELD_NAME) &&
      !sup::dto::TryConvert(anyvalue[CONNECTED_FIELD_NAME], ext_value.connected))
  {
    return false;
  }
  if (anyvalue.HasField(TIMESTAMP_FIELD_NAME) &&
      !sup::dto::TryConvert(anyvalue[TIMESTAMP_FIELD_NAME], ext_value.timestamp))
  {
    return false;
  }
  if (anyvalue.HasField(STATUS_FIELD_NAME) &&
      !sup::dto::TryConvert(anyvalue[STATUS_FIELD_NAME], ext_value.status))
  {
    return false;
  }
  if (anyvalue.HasField(SEVERITY_FIELD_NAME) &&
      !sup::dto::TryConvert(anyvalue[SEVERITY_FIELD_NAME], ext_value.severity))
  {
    return false;
  }
  return true;
}
}  // unnamed namespace
