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

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_ENCODED_CLIENT_VARIABLE_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_ENCODED_CLIENT_VARIABLE_H_

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
 * @brief Workspace variable associated with remote PvAccess server.
 * The variable value is encoded, which allows the type to be dynamic (the encoded result is always
 * of the same type). The encoding and its eventual type will be deduced from the 'encoding' field
 * in the PV (currently only base64 encoding is supported).
 * @code
     <Workspace>
       <PvAccessEncodedClient name="pvxs-variable"
         channel="seq::pvxs::variable"/>
     </Workspace>
   @endcode
 */
class PvAccessEncodedClientVariable : public Variable
{
public:
  PvAccessEncodedClientVariable();
  ~PvAccessEncodedClientVariable() override;

  static const std::string Type;

private:
  bool GetValueImpl(sup::dto::AnyValue &value) const override;
  bool SetValueImpl(const sup::dto::AnyValue &value) override;
  bool IsAvailableImpl() const override;
  SetupTeardownActions SetupImpl(const Workspace& ws) override;
  void TeardownImpl() override;

  std::unique_ptr<epics::PvAccessClientPV> m_pv;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_ENCODED_CLIENT_VARIABLE_H_
