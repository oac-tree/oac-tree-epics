/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : Instruction node implementation
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

// Global header files

#include <mutex> // std::mutex, etc.

#include <common/BasicTypes.h> // Misc. type definition
#include <common/StringTools.h> // Misc. helper functions
#include <common/TimeTools.h> // Misc. helper functions

#include <common/log-api.h> // Syslog wrapper routines

#include <common/PVMonitor.h> // Plug-in managed through dynamic linking .. see Makefile

#include <Variable.h>
#include <VariableRegistry.h>

// Local header files

#include "PVMonitorCache.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief PVMonitorVariable class.
 * @detail Workspace variable with asynchronous PVAccess monitoring. Mandatory attribute is the named
 * 'channel' (PV name) to connect to. The implementation allows for providing an optional 'status' attribute
 * specifying the name of an additional variable to host status information as part of the workspace.
 * @note Data access protection between concurrent calls to GetValue and SetValue is provided through the
 * Variable interface. Additional guard is provided between PVMonitorCache::HandleMonitor and
 * PVMonitorCache::GetValue.
 */

class PVMonitorVariable : public Variable, public PVMonitorCache
{

  private:

    /**
     * @brief Setup using provided attributes.
     */

    virtual bool Setup (void);

  protected:

  public:

    /**
     * @brief Constructor.
     */

    PVMonitorVariable (void);

    /**
     * @brief Destructor.
     */

    ~PVMonitorVariable (void) override;

    /**
     * @brief See sup::sequencer::Variable.
     */

    virtual bool GetValueImpl (ccs::types::AnyValue& value) const;
    virtual bool SetValueImpl (const ccs::types::AnyValue& value);

    /**
     * @brief Class name for VariableRegistry.
     */

    static const std::string Type;

};

// Function declaration

// Global variables

const std::string PVMonitorVariable::Type = "PVMonitorVariable";
static bool _pvmonitor_initialised_flag = RegisterGlobalVariable<PVMonitorVariable>();

// Function definition

bool PVMonitorVariable::Setup (void)
{

  bool status = ((false == PVMonitorCache::IsInitialised()) && Variable::HasAttribute("channel"));

  if (status)
    { // Instantiate implementation
      log_info("PVMonitorVariable('%s')::Setup - Method called with '%s' channel", Variable::GetName().c_str(), Variable::GetAttribute("channel").c_str());
      status = PVMonitorCache::SetChannel(Variable::GetAttribute("channel").c_str());
    }

  if (Variable::HasAttribute("status"))
    { // ToDo - Additional status variable
    }

  return status;

}

bool PVMonitorVariable::GetValueImpl (ccs::types::AnyValue& value) const
{

  bool status = PVMonitorCache::IsInitialised(); // Has received monitor and at least one valid value

  if (status)
    {
      status = PVMonitorCache::GetValue(value);
    }

  return status;

}

bool PVMonitorVariable::SetValueImpl (const ccs::types::AnyValue& value) { return false; } // Unsupported

PVMonitorVariable::PVMonitorVariable (void) : Variable(PVMonitorVariable::Type), PVMonitorCache() {}
PVMonitorVariable::~PVMonitorVariable (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
