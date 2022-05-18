/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : Variable plugin implementation
*
* Author        : Bertrand Bauvir (IO)
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

#include <common/BasicTypes.h> // Misc. type definition
#include <common/StringTools.h> // Misc. helper functions
#include <common/TimeTools.h> // Misc. helper functions

#include <common/log-api.h> // Syslog wrapper routines

#include <common/PVMonitor.h> // Plug-in managed through dynamic linking .. see Makefile

#include <Variable.h>
#include <VariableRegistry.h>

#include <memory>

#include "PVMonitorCache.h"

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

namespace sup {

namespace sequencer {

/**
 * @brief PVMonitorVariable class.
 * @detail Workspace variable with asynchronous PVAccess monitoring. Mandatory attribute is the named
 * 'channel' (PV name) to connect to.
 * @note Data access protection between concurrent calls to GetValue and SetValue is provided through the
 * Variable interface. Additional guard is provided between monitor_cache->HandleMonitor and
 * monitor_cache->GetValue.
 * @todo Assess if the implementation should allows for providing an optional 'status' attribute
 * specifying the name of an additional variable to host status information as part of the workspace.
 */

class PVMonitorVariable : public Variable
{
  private:
    std::unique_ptr<PVMonitorCache> monitor_cache;
    /**
     * @brief See sup::sequencer::Variable.
     */
    bool GetValueImpl (ccs::types::AnyValue& value) const override;
    bool SetValueImpl (const ccs::types::AnyValue& value) override;
    bool SetupImpl() override;
    void ResetImpl() override;

  protected:
  public:
    /**
     * @brief Constructor.
     */
    PVMonitorVariable();

    /**
     * @brief Destructor.
     */
    ~PVMonitorVariable() override;

    /**
     * @brief Class name for VariableRegistry.
     */
    static const std::string Type;
};

const std::string PVMonitorVariable::Type = "PVMonitorVariable";
static bool _pvmonitor_initialised_flag = RegisterGlobalVariable<PVMonitorVariable>();

PVMonitorVariable::PVMonitorVariable()
  : Variable(PVMonitorVariable::Type)
  , monitor_cache{new PVMonitorCache()}
{}

PVMonitorVariable::~PVMonitorVariable() = default;

bool PVMonitorVariable::GetValueImpl(ccs::types::AnyValue& value) const
{
  bool status = monitor_cache->IsInitialised(); // Has received monitor and at least one valid value
  if (status)
  {
    status = monitor_cache->GetValue(value);
  }
  return status;
}

bool PVMonitorVariable::SetValueImpl (const ccs::types::AnyValue& value) { return false; } // Unsupported

bool PVMonitorVariable::SetupImpl(void)
{
  bool status = ((false == monitor_cache->IsInitialised()) && HasAttribute("channel"));
  auto channel = GetAttribute("channel");

  if (status)
  {  // Instantiate implementation
    log_info("PVMonitorVariable('%s')::SetupImpl - Method called with '%s' channel",
             GetName().c_str(), channel.c_str());
    status = monitor_cache->SetChannel(channel.c_str());
  }
  if (status)
  {
    monitor_cache->SetCallback(channel.c_str(),
                [this](const ccs::types::AnyValue& value)
                {
                  Notify(value);
                  return;
                });
  }
  // ToDo - Additional status variable
  return status;
}

void PVMonitorVariable::ResetImpl()
{
  monitor_cache.reset(new PVMonitorCache());
}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
