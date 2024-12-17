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

#include "pv_access_helper.h"

#include <sup/dto/anyvalue_helper.h>

namespace sup
{
namespace sequencer
{
namespace pv_access_helper
{

sup::dto::AnyValue ConvertToTypedAnyValue(const sup::dto::AnyValue& value,
                                          const sup::dto::AnyType& anytype)
{
  sup::dto::AnyValue result{anytype};
  if (sup::dto::IsScalarType(anytype) && value.HasField(VALUE_FIELD_NAME))
  {
    if (sup::dto::TryConvert(result, value[VALUE_FIELD_NAME]))
    {
      return result;
    }
  }
  else
  {
    if (sup::dto::TryConvert(result, value))
    {
      return result;
    }
  }
  return {};
}

sup::dto::AnyValue PackIntoStructIfScalar(const sup::dto::AnyValue& value)
{
  if (!sup::dto::IsScalarValue(value))
  {
    return value;
  }
  sup::dto::AnyValue result = {{
    { VALUE_FIELD_NAME, value }
  }};
  return result;
}

}  // namespace pv_access_helper

}  // namespace sequencer

}  // namespace sup
