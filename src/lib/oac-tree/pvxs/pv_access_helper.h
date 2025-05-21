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

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_HELPER_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_HELPER_H_

#include <sup/dto/anyvalue.h>

namespace sup
{
namespace oac_tree
{
namespace pv_access_helper
{

const sup::dto::int64 DEFAULT_TIMEOUT_NS = 2000000000;  // 2 seconds

const std::string CHANNEL_ATTRIBUTE_NAME = "channel";
const std::string SERVICE_ATTRIBUTE_NAME = "service";
const std::string REQUEST_ATTRIBUTE_NAME = "requestVar";

const std::string VALUE_FIELD_NAME = "value";

sup::dto::AnyValue ConvertToTypedAnyValue(const sup::dto::AnyValue& value,
                                          const sup::dto::AnyType& anytype);

sup::dto::AnyValue PackIntoStructIfScalar(const sup::dto::AnyValue& value);

}  // namespace pv_access_helper

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_HELPER_H_
