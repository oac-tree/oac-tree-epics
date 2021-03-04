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
* Copyright (c) : 2010-2021 ITER Organization,
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

#include <common/BasicTypes.h> // Misc. type definition
#include <common/StringTools.h> // Misc. helper functions
#include <common/TimeTools.h> // Misc. helper functions

#include <common/log-api.h> // Syslog wrapper routines

#include <common/SharedReference.h>

#include <common/AnyTypeHelper.h>
#include <common/ChannelAccessClient.h>

#include <Variable.h>
#include <VariableRegistry.h>

// Local header files

#include "ToInteger.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief Workspace variable interface to EPICS Channel Access Process Variable (PV).
 * @details The class interfaces to ccs::base::ChannelAccessClient to manage asynchronous
 * handling of EPICS CA connections, notification and update. The variable is configured
 * with mandatory 'channel' (PV name) and 'datatype' attributes. The 'datatype' attribute
 * constraints the client-side type to use for get/put requests; e.g. enumeration-type
 * IOC records such as mbbi/mbbo can be accessed as integer or string. 
 * The implementation provides as well a way to change the CA client thread period from
 * default using an optional 'period' attribute with ns resolution.
 *
 * @code
     <Workspace>
       <ChannelAccessVariable name="boolean"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         datatype='{"type":"bool"}'
         period="10000000"/>
       <ChannelAccessVariable name="boolean-as-string"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         datatype='{"type":"string"}'/>
       <ChannelAccessVariable name="boolean-as-integer"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         datatype='{"type":"uint32"}'/>
       <ChannelAccessVariable name="integer"
         channel="EPICS::CA::CHANNEL::INTEGER"
         datatype='{"type":"uint32"}'/>
       <ChannelAccessVariable name="array"
         channel="EPICS::CA::CHANNEL::ARRAY"
         datatype='{"type":"uint32[]","multiplicity":8,"element":{"type":"uint32"}}'/>
     </Workspace>
   @endcode
 *
 * @note Multiple variable instances use the same singleton CA client instance which is
 * created and terminated as neceessary when variable instances come into procedure scope
 * and leave it.
 * @note The singleton CA client instance ignores new requests for already connected channels.
 * The example above would not currently run as such as the 'boolean-as-string' and 'boolean-as
 * -integer' would be ignored by the CA client implementation by reason of the channel name
 * to have already been registered. This implementation limitation may be lifted in the future
 * if required.
 * @note EPICS CA support is provided through this class and also as blocking instructions.
 * Procedures mixing asynchronous handling using this class and synchronous instructions have
 * not been tested.
 *
 * @todo Assess if a default instance value should be provided for the variable cache.
 * @todo Assess if an additional status variable should be provided to reflect to the
 * procedure if the channel is connected, etc.
 */

class ChannelAccessVariable : public Variable
{

  private:

    /**
     * @brief CA client reference attribute.
     */

    ccs::base::ChannelAccessClient* _client = NULL_PTR_CAST(ccs::base::ChannelAccessClient*);

  protected:

  public:

    /**
     * @brief Constructor.
     */

    ChannelAccessVariable (void);

    /**
     * @brief Destructor.
     */

    ~ChannelAccessVariable (void) override;

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

static inline ccs::base::ChannelAccessClient* GetChannelAccessClientInstance (void);
static inline bool LaunchChannelAccessClientInstance (void);
static inline bool TerminateChannelAccessClientInstance (void);

// Global variables

const std::string ChannelAccessVariable::Type = "ChannelAccessVariable";
static bool _cavariable_initialised_flag = RegisterGlobalVariable<ChannelAccessVariable>();

// ToDo - Replace the singleton mechanism in cpp-common-epics
static ccs::types::boolean _ca_client_launch = false;
static ccs::types::uint32 _ca_client_count = 0u;
static ccs::base::ChannelAccessClient* _ca_client = NULL_PTR_CAST(ccs::base::ChannelAccessClient*);

// Function definition

static inline ccs::base::ChannelAccessClient* GetChannelAccessClientInstance (void)
{

  bool status = (NULL_PTR_CAST(ccs::base::ChannelAccessClient*) != _ca_client);

  if (!status)
    { // Reference counted ccs::base::ChannelAccessClient instance
      log_notice("ChannelAccessClientContext::GetInstance - Initialise the shared reference ..");
      _ca_client = ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>();
    }

  // Increment count
  _ca_client_count += 1u;

  return _ca_client;

}

static inline bool LaunchChannelAccessClientInstance (void)
{

  bool status = (NULL_PTR_CAST(ccs::base::ChannelAccessClient*) != _ca_client);

  if (status && (false == _ca_client_launch))
    {
      log_notice("ChannelAccessClientContext::Launch - Starting CA caching thread ..");
      (void)_ca_client->Launch();
      (void)ccs::HelperTools::SleepFor(50000000ul);
      _ca_client_launch = true;
    }

  return status;

}

static inline bool TerminateChannelAccessClientInstance (void)
{

  bool status = (NULL_PTR_CAST(ccs::base::ChannelAccessClient*) != _ca_client);

  if (0u < _ca_client_count)
    { // Decrement count
      _ca_client_count -= 1u;
    }

  if (status && (0u == _ca_client_count))
    { // Assume destroying the reference is sufficient for context tear-down
      log_notice("ChannelAccessClientContext::Terminate - Forgetting the shared reference ..");
      ccs::base::ChannelAccessInterface::Terminate<ccs::base::ChannelAccessClient>();
      _ca_client = NULL_PTR_CAST(ccs::base::ChannelAccessClient*);
      _ca_client_launch = false;
    }

  return status;

}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
bool ChannelAccessVariable::SetupImpl (void)
{

  bool status = (Variable::HasAttribute("channel") && Variable::HasAttribute("datatype"));

  ccs::base::SharedReference<ccs::types::AnyType> _type; // Placeholder

  if (status)
    { // Instantiate required datatype
      log_debug("ChannelAccessVariable('%s')::SetupImpl - Method called with '%s' datatype ..", Variable::GetName().c_str(), Variable::GetAttribute("datatype").c_str());
      status = (0u < ccs::HelperTools::Parse(_type, Variable::GetAttribute("datatype").c_str()));
    }

  if (status)
    { // Instantiate variable cache
      _client = GetChannelAccessClientInstance();
      status = (NULL_PTR_CAST(ccs::base::ChannelAccessClient*) != _client);
    }

  if (status)
    { // Instantiate variable cache
      log_debug("ChannelAccessVariable('%s')::SetupImpl - .. and '%s' channel", Variable::GetName().c_str(), Variable::GetAttribute("channel").c_str());
      status = _client->AddVariable(Variable::GetAttribute("channel").c_str(), ccs::types::AnyputVariable, _type);
    }

  if (status && Variable::HasAttribute("period"))
    {
      ccs::types::uint64 _period = ::ccs::HelperTools::ToInteger<ccs::types::uint64>(Variable::GetAttribute("period").c_str());
      status = _client->SetPeriod(_period);
    }

  return status;

}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
bool ChannelAccessVariable::GetValueImpl (ccs::types::AnyValue& value) const
{
  // ToDo - Make an Instruction to start the CA variable cache with period ..
  (void)LaunchChannelAccessClientInstance(); // Only if not already done

  bool status = (NULL_PTR_CAST(ccs::base::ChannelAccessClient*) != _client);

  if (status)
    {
      status = _client->IsValid(Variable::GetAttribute("channel").c_str());
    }

  ccs::types::AnyValue* _value = NULL_PTR_CAST(ccs::types::AnyValue*);

  if (status)
    {
      _value = _client->GetVariable(Variable::GetAttribute("channel").c_str());
      status = (NULL_PTR_CAST(ccs::types::AnyValue*) != _value);
    }

  if (status)
    {
      value = *_value;
    }

  return status;

}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
bool ChannelAccessVariable::SetValueImpl (const ccs::types::AnyValue& value)
{
  // ToDo - Make an Instruction to start the CA variable cache with period ..
  (void)LaunchChannelAccessClientInstance(); // Only if not already done

  bool status = (NULL_PTR_CAST(ccs::base::ChannelAccessClient*) != _client);

  if (status)
    {
      status = _client->IsValid(Variable::GetAttribute("channel").c_str());
    }

  if (status)
    {
      status = _client->SetVariable(Variable::GetAttribute("channel").c_str(), value);
    }

  return status;

}

ChannelAccessVariable::ChannelAccessVariable (void) : Variable(ChannelAccessVariable::Type) {}
ChannelAccessVariable::~ChannelAccessVariable (void)
{ 

  bool status = (NULL_PTR_CAST(ccs::base::ChannelAccessClient*) != _client);

  if (status)
    { // Tear-down client context
      _client = NULL_PTR_CAST(ccs::base::ChannelAccessClient*); // Forget about it locally ..
      (void)TerminateChannelAccessClientInstance(); // .. last instance should finalise clean-up
    }

}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
