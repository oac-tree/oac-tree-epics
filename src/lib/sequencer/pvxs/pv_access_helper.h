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

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_HELPER_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_HELPER_H_

#include <sup/dto/anyvalue.h>

namespace sup
{
namespace sequencer
{
namespace pv_access_helper
{

const double DEFAULT_TIMEOUT_SEC = 2.0;

const std::string VALUE_FIELD_NAME = "value";

sup::dto::AnyValue ConvertToTypedAnyValue(const sup::dto::AnyValue& value,
                                          const sup::dto::AnyType& anytype);

sup::dto::AnyValue PackIntoStructIfScalar(const sup::dto::AnyValue& value);

}  // namespace pv_access_helper

}  // namespace sequencer

}  // namespace sup

#endif  // SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_HELPER_H_
