/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP - oac-tree
 *
 * Description   : oac-tree for operational procedures
 *
 * Author        : Walter Van Herck (IO)
 *
 * Copyright (c) : 2010-2025 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
******************************************************************************/

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_CHANNEL_ACCESS_HELPER_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_CHANNEL_ACCESS_HELPER_H_

#include <sup/dto/anyvalue.h>
#include <sup/epics/channel_access_pv.h>

#include <memory>

namespace sup
{
namespace oac_tree
{
namespace channel_access_helper
{
const double DEFAULT_TIMEOUT_SEC = 2.0;

const std::string VALUE_FIELD_NAME = "value";
const std::string CONNECTED_FIELD_NAME = "connected";
const std::string TIMESTAMP_FIELD_NAME = "timestamp";
const std::string STATUS_FIELD_NAME = "status";
const std::string SEVERITY_FIELD_NAME = "severity";

sup::dto::AnyType ChannelType(const sup::dto::AnyType& anytype);

sup::dto::AnyValue ExtractChannelValue(const sup::dto::AnyValue& value);

sup::dto::AnyValue ConvertToTypedAnyValue(
  const sup::epics::ChannelAccessPV::ExtendedValue& ext_value, const sup::dto::AnyType& anytype);

}  // namespace channel_access_helper

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_CHANNEL_ACCESS_HELPER_H_
