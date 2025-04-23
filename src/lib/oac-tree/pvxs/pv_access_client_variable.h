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

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_CLIENT_VARIABLE_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_CLIENT_VARIABLE_H_

#include <sup/oac-tree/variable.h>

#include <memory>

namespace sup
{
namespace epics
{
class PvAccessClientPV;
}  // namespace epics

namespace oac_tree
{
/**
 * @brief Workspace variable associated with remote pvAccess server.
 * The variable is configured with mandatory 'channel' (PV name) and 'type' attributes.
 * @code
     <Workspace>
       <PvAccessClient name="pvxs-variable"
         channel="seq::pvxs::variable"
         type='{"type":"seq::pvxs::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'/>
     </Workspace>
   @endcode
 */
class PvAccessClientVariable : public Variable
{
public:
  PvAccessClientVariable();
  ~PvAccessClientVariable();

  static const std::string Type;

private:
  bool GetValueImpl(sup::dto::AnyValue &value) const override;
  bool SetValueImpl(const sup::dto::AnyValue &value) override;
  bool IsAvailableImpl() const override;
  SetupTeardownActions SetupImpl(const Workspace& ws) override;
  void TeardownImpl() override;

  sup::dto::AnyType m_type;
  std::unique_ptr<epics::PvAccessClientPV> m_pv;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_CLIENT_VARIABLE_H_
