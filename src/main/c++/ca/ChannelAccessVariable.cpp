/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : Instruction node implementation
*
* Author        : B.Bauvir (IO)
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

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief Xxx
 * @todo Manage EPICS CA context as singleton and destroy when not necessary anylonger.
 * @todo Re-design to manage commonalities in separate class.
 * @todo StartChannelAccessCacheInstruction
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

static ccs::base::ChannelAccessClient* GetChannelAccessClientInstance (void);
static bool TerminateChannelAccessClientInstance (void);

// Global variables

const std::string ChannelAccessVariable::Type = "ChannelAccessVariable";
static bool _cavariable_initialised_flag = RegisterGlobalVariable<ChannelAccessVariable>();

// ToDo - Replace the singleton mechanism in cpp-common-epics
static ccs::types::boolean _ca_client_launch = false;
static ccs::types::uint32 _ca_client_count = 0u;
static ccs::base::ChannelAccessClient* _ca_client = NULL_PTR_CAST(ccs::base::ChannelAccessClient*);

// Function definition

static ccs::base::ChannelAccessClient* GetChannelAccessClientInstance (void)
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

static bool LaunchChannelAccessClientInstance (void)
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

static bool TerminateChannelAccessClientInstance (void)
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
#if 0
  if (status && Variable::HasAttribute("period"))
    { // ToDo
      ccs::types::uint64 _period = 100000000ul;
      status = _client->SetPeriod(_period);
    }
#endif
  return status;

}

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
#if 0 // Implicit with SetVariable .. would be required if copying memory
  if (status)
    {
      status = _client->UpdateVariable(Variable::GetAttribute("channel").c_str());
    }
#endif
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
