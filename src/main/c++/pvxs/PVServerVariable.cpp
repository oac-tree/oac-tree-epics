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

// Global header files

#include <mutex> // std::mutex, etc.

#include <common/BasicTypes.h> // Misc. type definition
#include <common/StringTools.h> // Misc. helper functions
#include <common/TimeTools.h> // Misc. helper functions

#include <common/log-api.h> // Syslog wrapper routines

#include <common/PVAccessServer.h>
#include <common/PVAccessTypes.h>

#include <Variable.h>
#include <VariableRegistry.h>

// Local header files

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief PVServerVariable class.
 * @detail Workspace variable associated with a locally hosted pvAccess server. Mandatory attribute is the named
 * 'channel' (PV name) to instantiate.
 */

class PVServerVariable : public Variable, public ::ccs::base::PVAccessServer
{

  private:

  protected:

  public:

    /**
     * @brief Constructor.
     */

    PVServerVariable (void);

    /**
     * @brief Destructor.
     */

    ~PVServerVariable (void) override;

    /**
     * @brief See sup::sequencer::Variable.
     */

    virtual bool SetupImpl (void);
    virtual bool GetValueImpl (ccs::types::AnyValue& value) const;
    virtual bool SetValueImpl (const ccs::types::AnyValue& value);

    /**
     * @brief Class name for VariableRegistry.
     */

    static const std::string Type;

};

// Function declaration

// Global variables

const std::string PVServerVariable::Type = "PVServerVariable";
static bool _pvserver_initialised_flag = RegisterGlobalVariable<PVServerVariable>();

// Function definition

bool PVServerVariable::SetupImpl (void)
{

  bool status = (Variable::HasAttribute("channel") && Variable::HasAttribute("datatype"));

  if (status)
    { // Make sure PVA types are registered (tests)
      status = ::ccs::base::PVAccessTypes::Initialise();
    }

  if (status)
    { // Instantiate implementation
      log_info("PVServerVariable('%s')::SetupImpl - Method called with '%s' channel", Variable::GetName().c_str(), Variable::GetAttribute("channel").c_str());
      status = ::ccs::base::PVAccessServer::AddVariable(Variable::GetAttribute("channel").c_str(), ::ccs::types::AnyputVariable, Variable::GetAttribute("datatype").c_str());
    }

  if (status)
    { // ToDo - Support more than one variable
      log_info("PVServerVariable('%s')::SetupImpl - Starting server", Variable::GetName().c_str());
      status = ::ccs::base::PVAccessServer::Launch();
    }

  if (status && Variable::HasAttribute("instance"))
    { // Provide default implementation
      status = ::ccs::base::PVAccessServer::GetVariable(Variable::GetAttribute("channel").c_str())->ParseInstance(Variable::GetAttribute("instance").c_str());
    }

  if (status)
    {
      status = ::ccs::base::PVAccessServer::UpdateVariable(Variable::GetAttribute("channel").c_str());
    }

  return status;

}

bool PVServerVariable::GetValueImpl (ccs::types::AnyValue& value) const
{

  bool status = ::ccs::base::PVAccessServer::IsValid(Variable::GetAttribute("channel").c_str());

  if (status)
    {
      status = ::ccs::base::PVAccessServer::GetVariable(Variable::GetAttribute("channel").c_str(), value);
    }

  return status;

}

bool PVServerVariable::SetValueImpl (const ccs::types::AnyValue& value)
{

  bool status = ::ccs::base::PVAccessServer::IsValid(Variable::GetAttribute("channel").c_str());

  if (status)
    {
      status = ::ccs::base::PVAccessServer::SetVariable(Variable::GetAttribute("channel").c_str(), value);
    }

  return status;

}

PVServerVariable::PVServerVariable (void) : Variable(PVServerVariable::Type), ::ccs::base::PVAccessServer() {}
PVServerVariable::~PVServerVariable (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
