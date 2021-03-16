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
 * 'channel' (PV name) to instantiate as well as the PV 'datatype'. An optional 'instance' attribute is used to
 * allow for providing a default value upon server start.
 * @code
     <Workspace>
       <PVServerVariable name="pvxs-variable"
         channel="seq::pvxs::variable"
         datatype='{"type":"seq::pvxs::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'
         instance='{"timestamp":0,"value":0.0}'/>
       <PVServerVariable name="other-variable" .. />
       <PVServerVariable name="yet-another-variable" .. />
     </Workspace>
   @endcode
 *
 * @note The singleton PVXS server instance is created and terminated as necessary. It is launched at the first call
 * to Variable::GetValue or SetValue interface to one of the workspace variables.
 *
 * @todo Highly structured variable datatypes would benefit from a mechanism by which types can be declared and loaded
 * in the GlobalTypeDatabase. E.g. by means of a RegisterType instructioon.
 */

class PVServerVariable : public Variable
{

  private:

    ccs::base::PVAccessServer* _server = NULL_PTR_CAST(ccs::base::PVAccessServer*);

    /**
     * @brief See sup::sequencer::Variable.
     */

    virtual bool SetupImpl (void);
    virtual bool GetValueImpl (ccs::types::AnyValue& value) const;
    virtual bool SetValueImpl (const ccs::types::AnyValue& value);

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
     * @brief Class name for VariableRegistry.
     */

    static const std::string Type;

};

// Function declaration

static ccs::base::PVAccessServer* GetPVAccessServerInstance (void);
static bool LaunchPVAccessServerInstance (void);
static bool TerminatePVAccessServerInstance (void);

// Global variables

const std::string PVServerVariable::Type = "PVServerVariable";
static bool _pvserver_initialised_flag = RegisterGlobalVariable<PVServerVariable>();

// ToDo - Replace the singleton mechanism in cpp-common-epics
static ccs::types::boolean _pvxs_server_launch = false;
static ccs::types::uint32 _pvxs_server_count = 0u;
static ccs::base::PVAccessServer* _pvxs_server = NULL_PTR_CAST(ccs::base::PVAccessServer*);

// Function definition

static ccs::base::PVAccessServer* GetPVAccessServerInstance (void)
{

  bool status = (NULL_PTR_CAST(ccs::base::PVAccessServer*) != _pvxs_server);

  if (!status)
    { // Reference counted instance
      log_notice("PVAccessServerContext::GetInstance - Initialise the shared reference ..");
      _pvxs_server = ccs::base::PVAccessInterface::GetInstance<ccs::base::PVAccessServer>();
    }

  // Increment count
  _pvxs_server_count += 1u;

  return _pvxs_server;

}

static bool LaunchPVAccessServerInstance (void)
{

  bool status = (NULL_PTR_CAST(ccs::base::PVAccessServer*) != _pvxs_server);

  if (status && (false == _pvxs_server_launch))
    {
      log_notice("PVAccessServerContext::Launch - Starting PVXS server thread ..");
      (void)_pvxs_server->Launch();
      (void)ccs::HelperTools::SleepFor(50000000ul);
      _pvxs_server_launch = true;
    }

  return status;

}

static bool TerminatePVAccessServerInstance (void)
{

  bool status = (NULL_PTR_CAST(ccs::base::PVAccessServer*) != _pvxs_server);

  if (0u < _pvxs_server_count)
    { // Decrement count
      _pvxs_server_count -= 1u;
    }

  if (status && (0u == _pvxs_server_count))
    { // Assume destroying the reference is sufficient for context tear-down
      log_notice("PVAccessServerContext::Terminate - Forgetting the shared reference ..");
      ccs::base::PVAccessInterface::Terminate<ccs::base::PVAccessServer>();
      _pvxs_server = NULL_PTR_CAST(ccs::base::PVAccessServer*);
      _pvxs_server_launch = false;
    }

  return status;

}

// Function definition

bool PVServerVariable::SetupImpl (void)
{

  bool status = (Variable::HasAttribute("channel") && Variable::HasAttribute("datatype"));

  if (status)
    { // Make sure PVA types are registered (tests)
      status = ::ccs::base::PVAccessTypes::Initialise();
    }

  if (status)
    { // Instantiate variable server ..  as necessary
      _server = GetPVAccessServerInstance();
      status = (NULL_PTR_CAST(ccs::base::PVAccessServer*) != _server);
    }

  if (status)
    { // Instantiate implementation
      log_info("PVServerVariable('%s')::SetupImpl - Method called with '%s' channel", Variable::GetName().c_str(), Variable::GetAttribute("channel").c_str());
      status = _server->AddVariable(Variable::GetAttribute("channel").c_str(), ::ccs::types::AnyputVariable, Variable::GetAttribute("datatype").c_str());
    }

  if (status && Variable::HasAttribute("instance"))
    { // Provide default instance
      status = _server->GetVariable(Variable::GetAttribute("channel").c_str())->ParseInstance(Variable::GetAttribute("instance").c_str());
    }

  if (status)
    {
      status = _server->UpdateVariable(Variable::GetAttribute("channel").c_str());
    }

  return status;

}

bool PVServerVariable::GetValueImpl (ccs::types::AnyValue& value) const
{

  // ToDo - Make an Instruction to start the PVXS variable server with period ..
  (void)LaunchPVAccessServerInstance(); // Only if not already done

  bool status = _server->IsValid(Variable::GetAttribute("channel").c_str());

  if (status)
    {
      status = _server->GetVariable(Variable::GetAttribute("channel").c_str(), value);
    }

  return status;

}

bool PVServerVariable::SetValueImpl (const ccs::types::AnyValue& value)
{

  // ToDo - Make an Instruction to start the PVXS variable server with period ..
  (void)LaunchPVAccessServerInstance(); // Only if not already done

  bool status = _server->IsValid(Variable::GetAttribute("channel").c_str());

  if (status)
    {
      status = _server->SetVariable(Variable::GetAttribute("channel").c_str(), value);
    }

  return status;

}

PVServerVariable::PVServerVariable (void) : Variable(PVServerVariable::Type) {}
PVServerVariable::~PVServerVariable (void)
{

  bool status = (NULL_PTR_CAST(ccs::base::PVAccessServer*) != _server);

  if (status)
    { // Tear-down server context
      _server = NULL_PTR_CAST(ccs::base::PVAccessServer*); // Forget about it locally ..
      (void)TerminatePVAccessServerInstance(); // .. last instance should finalise clean-up
    }

}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
