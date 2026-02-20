/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Walter Van Herck (IO)
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

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_ENCODED_SERVER_VARIABLE_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_ENCODED_SERVER_VARIABLE_H_

#include "pv_access_shared_server.h"

#include <sup/oac-tree/variable.h>

#include <memory>

namespace sup
{
namespace epics
{
class PvAccessServer;
}  // namespace epics

namespace oac_tree
{
/**
 * @brief Workspace variable associated with a locally hosted pvAccess server.
 * The variable will be presented on the network as an encoded variable. This makes its type
 * dynamic. It has the following attributes:
 * - channel: mandatory name of PvAccess channel
 * - encoding: optional encoding name (default is base64)
 * - type: optional type for the initial value
 * - value: optional initial value
 * @code
     <Workspace>
       <PvAccessEncodedServer name="pvxs-variable"
         channel="seq::pvxs::servervariable"
         encoding="base64"
         type='{"type":"seq::pvxs::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'/>
     </Workspace>
   @endcode
 */
class PvAccessEncodedServerVariable : public Variable
{
public:
  PvAccessEncodedServerVariable();
  ~PvAccessEncodedServerVariable() override;

  static const std::string Type;

private:
  PvAccessSharedServer& GetSharedServer() const;
  bool GetValueImpl(sup::dto::AnyValue &value) const override;
  bool SetValueImpl(const sup::dto::AnyValue &value) override;
  bool IsAvailableImpl() const override;
  SetupTeardownActions SetupImpl(const Workspace& ws) override;
  void ResetImpl(const Workspace& ws) override;
  void TeardownImpl() override;

  sup::dto::AnyType m_initial_type;
  const Workspace* m_workspace;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_ENCODED_SERVER_VARIABLE_H_
