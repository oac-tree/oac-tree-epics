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
#if 0
    /**
     * @brief CA client reference attribute.
     */

    chid _channel;

    /**
     * @brief PV copy.
     */

    ccs::types::AnyValue _value;
#endif
    /**
     * @brief Setup using provided attributes.
     */

    virtual bool Setup (void);

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

    virtual bool GetValueImpl (ccs::types::AnyValue& value) const;
    virtual bool SetValueImpl (const ccs::types::AnyValue& value);

    /**
     * @brief Class name for VariableRegistry.
     */

    static const std::string Type;

};

// Function declaration

// Global variables

const std::string ChannelAccessVariable::Type = "ChannelAccessVariable";
static bool _cavariable_initialised_flag = RegisterGlobalVariable<ChannelAccessVariable>();

// Function definition

bool ChannelAccessVariable::Setup (void)
{

  bool status = (Variable::HasAttribute("channel") && Variable::HasAttribute("datatype"));

  ccs::base::SharedReference<ccs::types::AnyType> _type; // Placeholder

  if (status)
    { // Instantiate required datatype
      log_info("ChannelAccessVariable('%s')::Setup - Method called with '%s' datatype ..", Variable::GetName().c_str(), Variable::GetAttribute("datatype").c_str());
      status = (0u < ccs::HelperTools::Parse(_type, Variable::GetAttribute("datatype").c_str()));
    }

  if (status)
    { // Instantiate variable cache
      log_info("ChannelAccessVariable('%s')::Setup - .. and '%s' channel", Variable::GetName().c_str(), Variable::GetAttribute("channel").c_str());
      status = ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->AddVariable(Variable::GetAttribute("channel").c_str(), ccs::types::AnyputVariable, _type);
    }
#if 0
  if (status && Variable::HasAttribute("period"))
    { // ToDo
      ccs::types::uint64 _period = 100000000ul;
      status = ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->SetPeriod(_period);
    }
#endif
  return status;

}

bool ChannelAccessVariable::GetValueImpl (ccs::types::AnyValue& value) const
{
  // ToDo - Make an Instruction to start the CA variable cache with period ..
  (void)ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->Launch();

  bool status = ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->IsValid(Variable::GetAttribute("channel").c_str());

  ccs::types::AnyValue* _value = NULL_PTR_CAST(ccs::types::AnyValue*);

  if (status)
    {
      _value = ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->GetVariable(Variable::GetAttribute("channel").c_str());
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
  (void)ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->Launch();

  bool status = ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->IsValid(Variable::GetAttribute("channel").c_str());

  if (status)
    {
      status = ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->SetVariable(Variable::GetAttribute("channel").c_str(), value);
    }
#if 0
  if (status)
    {
      status = ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->UpdateVariable(Variable::GetAttribute("channel").c_str());
    }
#endif
  return status;

}

ChannelAccessVariable::ChannelAccessVariable (void) : Variable(ChannelAccessVariable::Type) {}
ChannelAccessVariable::~ChannelAccessVariable (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
