/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : System clock variable implementation
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

#include <common/BasicTypes.h> // Misc. type definition
#include <common/TimeTools.h> // Misc. helper functions

#include <common/AnyValueHelper.h> // Misc. helper functions

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
 * @brief SystemClockVariable class.
 * @details Returns current time when Variable::GetValueImpl is being called.
 * @note Variable::SetValueImpl is not supported and returns false if called.
 * @todo Assess if different format should be supported, e.g. ISO8601 is the passed value
 * is string, etc.
 */

class SystemClockVariable : public Variable
{

  private:

  protected:

  public:

    /**
     * @brief Constructor.
     */

    SystemClockVariable (void);

    /**
     * @brief Destructor.
     */

    ~SystemClockVariable (void) override;

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

const std::string SystemClockVariable::Type = "SystemClock";
static bool _sysclockvariable_initialised_flag = RegisterGlobalVariable<SystemClockVariable>();

// Function definition

bool SystemClockVariable::GetValueImpl (ccs::types::AnyValue& value) const
{

  bool status = static_cast<bool>(value.GetType());

  if (!status)
    {
      value = ccs::types::AnyValue (ccs::types::UnsignedInteger64);
      status = static_cast<bool>(value.GetType()); 
    }

  if (status)
    {
      status = (ccs::types::UnsignedInteger64 == value.GetType());
    }

  if (status)
    {
      value = ::ccs::HelperTools::GetCurrentTime();
    }

  return status;

}

bool SystemClockVariable::SetValueImpl (const ccs::types::AnyValue& value) { return false; } // Unimplemented

SystemClockVariable::SystemClockVariable (void) : Variable(SystemClockVariable::Type) {}
SystemClockVariable::~SystemClockVariable (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
