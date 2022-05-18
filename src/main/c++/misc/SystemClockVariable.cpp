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
    ::ccs::base::SharedReference<const ::ccs::types::AnyType> _type;

    /**
     * @brief See sup::sequencer::Variable.
     */
    virtual bool GetValueImpl (::ccs::types::AnyValue& value) const;
    virtual bool SetValueImpl (const ::ccs::types::AnyValue& value);
    virtual bool SetupImpl();
    virtual void ResetImpl();

  protected:
  public:
    /**
     * @brief Constructor.
     */
    SystemClockVariable();

    /**
     * @brief Destructor.
     */
    ~SystemClockVariable() override;

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

  ccs::types::uint64 _time = ::ccs::HelperTools::GetCurrentTime();

  bool status = static_cast<bool>(_type);

  ccs::types::AnyValue _value; // Placeholder

  if (status)
    {
      _value = ccs::types::AnyValue (_type);
    }

  if (ccs::types::UnsignedInteger64 == _type)
    {
      _value = _time;
    }

  if (ccs::types::String == _type)
    {
      ::ccs::HelperTools::ToISO8601(_time, static_cast<ccs::types::char8*>(_value.GetInstance()), _type->GetSize());
    }

  if (std::string(_type->GetName()) == "sup::FractionalTime/v1.0")
    {
      struct { ::ccs::types::uint32 secs; ::ccs::types::uint32 nsec; } _fract = { 0u, 0u};
      _fract.secs = static_cast<::ccs::types::uint32>(_time / 1000000000ul);
      _fract.nsec = static_cast<::ccs::types::uint32>(_time - (1000000000ul * static_cast<::ccs::types::uint64>(_fract.secs)));
      _value = _fract;
    }

  if (status)
    {
      if ((false == static_cast<bool>(value.GetType())) ||
          ((value.GetType())->GetSize() == _type->GetSize()))
        {
          value = _value;
          status = static_cast<bool>(value.GetType());
        }
    }

  return status;

}

bool SystemClockVariable::SetValueImpl(const ccs::types::AnyValue&)
{
  return false;
}

bool SystemClockVariable::SetupImpl (void)
{

  if (Variable::HasAttribute("datatype")) // Has 'datatype' attribute
    {
      ::ccs::base::SharedReference<::ccs::types::AnyType> type;
      bool status = (0u < ::ccs::HelperTools::Parse(type, Variable::GetAttribute("datatype").c_str()));

      if (status)
        { // Now going to const type
          _type = type;
        }
    }
  else
    { // Default to 'uint64' type
      _type = ::ccs::types::UnsignedInteger64;
    }

  return static_cast<bool>(_type);
}

void SystemClockVariable::ResetImpl()
{
  _type.Discard();
}


SystemClockVariable::SystemClockVariable() : Variable(SystemClockVariable::Type)
{
  // Register timespec equivalent type to the GlobalTypeDatabase
  ::ccs::types::char8 type [] = "{\"type\":\"sup::FractionalTime/v1.0\",\"attributes\":["
    "{\"seconds\":{\"type\":\"uint32\"}},"
    "{\"nanosec\":{\"type\":\"uint32\"}}"
    "]}";
  (void)::ccs::base::GlobalTypeDatabase::Register(type);
}

SystemClockVariable::~SystemClockVariable() = default;

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
