/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Walter Van Herck (IO)
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

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_SERVER_VARIABLE_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_SERVER_VARIABLE_H_

#include <sup/sequencer/variable.h>

#include <memory>

namespace sup
{
namespace epics
{
class PvAccessServer;
}  // namespace epics

namespace sequencer
{
/**
 * @brief Workspace variable associated with a locally hosted pvAccess server.
 * The variable is configured with mandatory 'channel' and 'type' attributes. An initial value can
 * be provided with the optional 'value' attribute.
 * @code
     <Workspace>
       <PvAccessServer name="pvxs-variable"
         channel="seq::pvxs::servervariable"
         type='{"type":"seq::pvxs::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'/>
     </Workspace>
   @endcode
 */
class PvAccessServerVariable : public Variable
{
public:
  PvAccessServerVariable();
  ~PvAccessServerVariable();

  static const std::string Type;

private:
  bool GetValueImpl(sup::dto::AnyValue &value) const override;
  bool SetValueImpl(const sup::dto::AnyValue &value) override;
  bool IsAvailableImpl() const override;
  void SetupImpl(const Workspace& ws) override;
  void ResetImpl() override;

  sup::dto::AnyType m_type;
  std::unique_ptr<epics::PvAccessServer> m_server;
};

}  // namespace sequencer

}  // namespace sup

#endif  // SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_SERVER_VARIABLE_H_
