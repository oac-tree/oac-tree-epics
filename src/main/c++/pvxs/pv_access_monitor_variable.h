/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Walter Van Herck
 *
 * Copyright (c) : 2010-2022 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_MONITOR_VARIABLE_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_MONITOR_VARIABLE_H_

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
 * @brief Read-only workspace variable associated with remote pvAccess server.
 * The variable is configured with mandatory 'channel' (PV name) and optional 'type' attributes.
 * @code
     <Workspace>
       <PvAccessMonitor name="pvxs-variable"
         channel="seq::pvxs::variable"
         type='{"type":"seq::pvxs::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'/>
     </Workspace>
   @endcode
 */
class PvAccessMonitorVariable : public Variable
{
public:
  PvAccessMonitorVariable();
  ~PvAccessMonitorVariable();

  static const std::string Type;

private:
  bool GetValueImpl(sup::dto::AnyValue &value) const override;
  bool SetValueImpl(const sup::dto::AnyValue &value) override;
  bool IsAvailableImpl() const override;
  bool SetupImpl(const sup::dto::AnyTypeRegistry& registry) override;
  void ResetImpl() override;

  std::unique_ptr<sup::dto::AnyType> m_type;
  std::unique_ptr<epics::PvAccessClientPV> m_pv;
};

}  // namespace sequencer

}  // namespace sup

#endif  // SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_MONITOR_VARIABLE_H_
