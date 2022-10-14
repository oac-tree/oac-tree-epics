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

#include <sup/sequencer/variable.h>

#include <memory>

namespace sup
{
namespace epics
{
class PvAccessClientPV;
}  // namespace epics

namespace sequencer
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
  bool SetupImpl(const sup::dto::AnyTypeRegistry& registry) override;
  void ResetImpl() override;

  std::unique_ptr<epics::PvAccessClientPV> m_pv;
  sup::dto::AnyType m_type;
};

}  // namespace sequencer

}  // namespace sup
