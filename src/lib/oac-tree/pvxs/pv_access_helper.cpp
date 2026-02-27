/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Gennady Pospelov
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

#include "pv_access_helper.h"

#include <sup/dto/anyvalue_helper.h>

#include <deque>

namespace sup
{
namespace oac_tree
{
namespace pv_access_helper
{
namespace
{
// Try to convert the src value to dest, allowing for dest to only contain a subset of fields of
// src. dest is assumed to be non-empty and non-scalar.
bool TryPartialConvert(sup::dto::AnyValue& dest, const sup::dto::AnyValue& src);
}  // unnamed namespace

sup::dto::AnyValue ConvertToTypedAnyValue(const sup::dto::AnyValue& value,
                                          const sup::dto::AnyType& anytype)
{
  if (sup::dto::IsEmptyType(anytype))
  {
    return value;
  }
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

PvAccessSharedServerRegistry& GetSharedPvAccessServerRegistry()
{
  static PvAccessSharedServerRegistry shared_registry{};
  return shared_registry;
}

namespace
{

struct PartialConvertNode
{
  sup::dto::AnyValue* m_dest;
  const sup::dto::AnyValue* m_src;
};

bool TryPartialConvert(sup::dto::AnyValue& dest, const sup::dto::AnyValue& src)
{
  // Prevent failure after partial updates:
  auto copy = dest;
  std::deque<PartialConvertNode> stack{};
  stack.push_back(PartialConvertNode{std::addressof(copy), std::addressof(src)});
  while (!stack.empty())
  {
    auto& front = stack.front();
    auto* current_dest = front.m_dest;
    auto* current_src = front.m_src;
    if (sup::dto::IsStructValue(*current_dest))
    {
      auto mem_names = current_dest->MemberNames();
      for (const auto& mem_name : mem_names)
      {
        if (!current_src->HasField(mem_name))
        {
          return false;
        }
        auto* dest_mem = std::addressof((*current_dest)[mem_name]);
        auto* src_mem = std::addressof((*current_src)[mem_name]);
        stack.push_back(PartialConvertNode{dest_mem, src_mem});
      }
    }
    else
    {
      if (!sup::dto::TryConvert(*current_dest, *current_src))
      {
        return false;
      }
    }
    stack.pop_front();
  }
  dest = copy;
  return true;
}

}  // unnamed namespace

}  // namespace pv_access_helper

}  // namespace oac_tree

}  // namespace sup
