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

#include <Variable.h>

namespace sup
{
namespace sequencer
{
/**
 * @brief Workspace variable associated with remote pvAccess server.
 * The variable is configured with mandatory 'channel' (PV name) and 'datatype' attributes.
 * @code
     <Workspace>
       <PVClientVariable name="pvxs-variable"
         channel="seq::pvxs::variable"
         datatype='{"type":"seq::pvxs::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'/>
     </Workspace>
   @endcode
 */

class PVClientVariable : public Variable
{
public:
  PVClientVariable();
  ~PVClientVariable();

  static const std::string Type;

private:
  bool SetupImpl() override;
  bool GetValueImpl(ccs::types::AnyValue &value) const override;
  bool SetValueImpl(const ccs::types::AnyValue &value) override;

  struct PVClientVariableImpl;
  PVClientVariableImpl *p_impl;
};

}  // namespace sequencer

}  // namespace sup
