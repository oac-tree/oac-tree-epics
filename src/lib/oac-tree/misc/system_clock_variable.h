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
 * SPDX-License-Identifier: MIT
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file LICENSE located in the top level directory
 * of the distribution package.
******************************************************************************/

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_SYSTEM_CLOCK_VARIABLE_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_SYSTEM_CLOCK_VARIABLE_H_

#include <sup/oac-tree/variable.h>

namespace sup
{
namespace oac_tree
{

/**
 * @brief SystemClockVariable class.
 *
 * @details Returns current time when its value is retrieved.
 */
class SystemClockVariable : public Variable
{
public:
  SystemClockVariable();
  ~SystemClockVariable() override;

  static const std::string Type;

private:
  bool GetValueImpl(sup::dto::AnyValue& value) const override;
  bool SetValueImpl(const sup::dto::AnyValue& value) override;
  SetupTeardownActions SetupImpl(const Workspace& ws) override;
  void TeardownImpl() override;
  std::string m_time_format;

  std::unique_ptr<sup::dto::AnyValue> ReadCurrentTime() const;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_SYSTEM_CLOCK_VARIABLE_H_
