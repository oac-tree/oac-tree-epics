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
    ccs::base::ChannelAccessClient client;
    ccs::base::SharedReference<const ccs::types::AnyType> var_type;
    std::string channel;

    bool SetupImpl() override;
    bool GetValueImpl(ccs::types::AnyValue& value) const override;
    bool SetValueImpl(const ccs::types::AnyValue& value) override;

  public:
    ChannelAccessVariable();
    ~ChannelAccessVariable() override;

    /**
     * @brief Class name for VariableRegistry.
     */
    static const std::string Type;
};

// Global variables

const std::string ChannelAccessVariable::Type = "ChannelAccessVariable";
static bool _cavariable_registered_flag = RegisterGlobalVariable<ChannelAccessVariable>();

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
bool ChannelAccessVariable::SetupImpl()
{
  bool status = (HasAttribute("channel") && HasAttribute("datatype"));

  ccs::base::SharedReference<ccs::types::AnyType> type;

  if (status)
  { // Instantiate required datatype
    log_debug("ChannelAccessVariable('%s')::SetupImpl - Method called with '%s' datatype ..", GetName().c_str(), GetAttribute("datatype").c_str());
    channel = GetAttribute("channel");
    status = (0u < ccs::HelperTools::Parse(type, GetAttribute("datatype").c_str()));
  }
  if (status)
  { // Instantiate variable cache
    var_type = type;
    log_debug("ChannelAccessVariable('%s')::SetupImpl - .. and '%s' channel", GetName().c_str(), GetAttribute("channel").c_str());
    status = client.AddVariable(channel.c_str(), ccs::types::AnyputVariable, var_type);
  }
  return status;
}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
bool ChannelAccessVariable::GetValueImpl(ccs::types::AnyValue &value) const
{
  bool status = client.IsValid(channel.c_str()) && static_cast<bool>(var_type);

  if (status)
  {
    value = ccs::types::AnyValue(var_type);
    if (!client.GetExtendedVariable(channel.c_str(), value))
    {
      status = client.GetAnyValue(channel.c_str(), value);
    }
  }
  return status;
}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
bool ChannelAccessVariable::SetValueImpl(const ccs::types::AnyValue &value)
{
  bool status = client.IsValid(channel.c_str());

  if (status)
  {
    status = client.SetAnyValue(channel.c_str(), value);
  }
  return status;
}

ChannelAccessVariable::ChannelAccessVariable()
  : Variable(ChannelAccessVariable::Type)
{}

ChannelAccessVariable::~ChannelAccessVariable()
{}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
