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
*                                 CS 90 046
*                                 13067 St. Paul-lez-Durance Cedex
*                                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

// Global header files

#include <algorithm> // std::find, etc.
#include <mutex> // std::mutex, etc.

#include <common/BasicTypes.h> // Misc. type definition
#include <common/StringTools.h> // Misc. helper functions
#include <common/TimeTools.h> // Misc. helper functions

//#include <common/ToInteger.h>

#include <common/log-api.h> // Syslog wrapper routines

#include <common/AnyValue.h>
#include <common/AnyValueHelper.h>

//#include <common/ChannelAccessClientContext.h>
#include <common/ChannelAccessHelper.h> // CA helper routines
#include <common/AnyTypeToCA.h> // CA helper routines .. type conversion

#include <ExecutionStatus.h>

#include <Instruction.h>
#include <InstructionRegistry.h>

#include <Procedure.h>
#include <Workspace.h>

// Local header files

#include "ToInteger.h"

#include "ChannelAccessClientContext.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

//#define ChannelAccessInstruction_Use_Mutex
//#define ChannelAccessInstruction_Share_Context

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief Implementation base class.
 * @details Handles channel connection and disconnection which is common for
 * read/write operations.
 */

class ChannelAccessInstructionHelper
{

  private:

    /**
     * @brief CA channel identifier attribute.
     */

    chid _channel;
    bool _connected = false;

    ccs::types::uint64 _delay = 100000000ul;

  protected:

  public:

    bool HandleConnect (const ccs::types::char8 * const channel);
    bool HandleDetach (void);
    chid GetChannel (void) const;

    bool SetDelay (ccs::types::uint64 delay);
    bool SetDelay (const ccs::types::char8 * const delay);

    bool VerifyValue (const ccs::types::AnyValue& value) const;

    bool ReadChannel (ccs::types::AnyValue& value) const;
    bool WriteChannel (const ccs::types::AnyValue& value) const;

    /**
     * @brief Constructor.
     */

    ChannelAccessInstructionHelper (void);

    /**
     * @brief Destructor.
     */

    virtual ~ChannelAccessInstructionHelper (void);

};

/**
 * @brief Instruction interfacing to EPICS Channel Access Process Variable (PV).
 * @details The class provides a blocking read to EPICS CA. The instruction fails
 * in case the configured 'channel' can not be accessed. Upon successful read, the
 * specified workspace 'variable' is updated. The datatype of the workspace variable
 * defines how the client-side tries and read the remove channel.
 *
 * @code
     <Sequence>
       <ChannelAccessFetchInstruction name="get-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         variable="boolean"/>
       <ChannelAccessFetchInstruction name="get-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         variable="uint32"/>
       <ChannelAccessFetchInstruction name="get-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         variable="string"/>
     </Sequence>
     <Workspace>
       <Local name="boolean"
         type='{"type":"bool"}'
         value="false"/>
       <Local name="uint32"
         type='{"type":"uint32"}'
         value="0"/>
       <Local name="string"
         type='{"type":"string"}'
         value='"undefined"'/>
     </Workspace>
   @endcode
 *
 * @note EPICS CA support is provided through this class and also as asynchronous variables.
 * Procedures mixing asynchronous handling and synchronous instructions have not been tested.
 * @note A single EPICS CA context is created for the sequencer procedure and shared among
 * all instruction instances. An explicit context attach/detach is performed by each call to
 * Instruction::ExecuteSingleImpl in order to allow for multi-threaded operation.
 */

class ChannelAccessFetchInstruction : public Instruction, public ChannelAccessInstructionHelper
{

  private:

    /**
     * @brief Workspace variable copy.
     */

    ccs::types::AnyValue _value;

    /**
     * @brief See sup::sequencer::Instruction.
     * @details Verify and handle attributes.
     */

    virtual bool SetupImpl (const Procedure& proc);

    /**
     * @brief See sup::sequencer::Instruction.
     * @details Connects to the specified 'channel' and read the value into the
     * workspace 'variable'.
     */

    ExecutionStatus ExecuteSingleImpl (UserInterface * ui, Workspace * ws) override;

  protected:

  public:

    /**
     * @brief Constructor.
     */

    ChannelAccessFetchInstruction (void);

    /**
     * @brief Destructor.
     */

    ~ChannelAccessFetchInstruction (void) override;

    /**
     * @brief Class name for InstructionRegistry.
     */

    static const std::string Type;

};

/**
 * @brief Instruction interfacing to EPICS Channel Access Process Variable (PV).
 * @details The class provides a blocking write to EPICS CA. The instruction fails
 * in case the configured 'channel' can not be accessed. The instruction provides
 * two ways EPICS CA channels are updated:
 *
 *   Using 'datatype' and 'instance' specification through attributes, or
 *   By reference to a workspace 'variable' holding the value to be written.
 *
 * The EPICS CA connection is verified after an optional 'delay' period specified in
 * ns resolution which is defaulted to 100ms.
 * @code
     <Sequence>
       <ChannelAccessWriteInstruction name="put-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         delay="100000000"
         variable="boolean"/>
       <ChannelAccessFetchInstruction name="put-as-integer"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         datatype='{"type":"uint32"}'
         instance="0"/>
       <ChannelAccessFetchInstruction name="put-as-string"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         datatype='{"type":"string"}'
         instance='"FALSE"'/> <!-- As appropriate as per record specification --> <!-- Note the quotes -->
     </Sequence>
     <Workspace>
       <Local name="boolean"
         type='{"type":"bool"}'
         value="false"/>
     </Workspace>
   @endcode
 *
 * @note EPICS CA support is provided through this class and also as asynchronous variables.
 * Procedures mixing asynchronous handling and synchronous instructions have not been tested.
 * @note A single EPICS CA context is created for the sequencer procedure and shared among
 * all instruction instances. An explicit context attach/detach is performed by each call to
 * Instruction::ExecuteSingleImpl in order to allow for multi-threaded operation.
 */

class ChannelAccessWriteInstruction : public Instruction, public ChannelAccessInstructionHelper
{

  private:

    /**
     * @brief Workspace variable copy.
     */

    ccs::types::AnyValue _value;

    /**
     * @brief See sup::sequencer::Instruction.
     * @details Verify and handle attributes.
     */

    bool VerifyAttributes (const Procedure& proc) const;

    /**
     * @brief See sup::sequencer::Instruction.
     * @details Verify and handle attributes.
     */

    virtual bool SetupImpl (const Procedure& proc);

    /**
     * @brief See sup::sequencer::Instruction.
     */

    ExecutionStatus ExecuteSingleImpl (UserInterface * ui, Workspace * ws) override;

  protected:

  public:

    /**
     * @brief Constructor.
     */

    ChannelAccessWriteInstruction (void);

    /**
     * @brief Destructor.
     */

    ~ChannelAccessWriteInstruction (void) override;

    /**
     * @brief Class name for InstructionRegistry.
     */

    static const std::string Type;

};

// Function declaration

// Global variables

const std::string ChannelAccessFetchInstruction::Type = "ChannelAccessFetchInstruction";
const std::string ChannelAccessWriteInstruction::Type = "ChannelAccessWriteInstruction";

static bool _caclient_initialised_flag = (RegisterGlobalInstruction<ChannelAccessFetchInstruction>() && RegisterGlobalInstruction<ChannelAccessWriteInstruction>());

/**
 * @brief Mutex for concurrent access of CA helper routines.
 */
#ifdef ChannelAccessInstruction_Use_Mutex
static std::mutex _async_mutex;
#endif
// Function definition

bool ChannelAccessInstructionHelper::HandleConnect (const ccs::types::char8 * const channel)
{

  log_debug("ChannelAccessInstructionHelper::HandleConnect - Method called for '%s'", channel);

  bool status = (false == _connected);

  if (!status)
    {
      log_warning("ChannelAccessInstructionHelper::HandleConnect('%s') - Channel already connected ..", channel);
    }

  if (status)
    { // Attach to CA context .. implicit create if necessary
#ifdef ChannelAccessInstruction_Share_Context
      status = ::ccs::HelperTools::ChannelAccessClientContext::Attach();
#else
      status = ::ccs::HelperTools::ChannelAccess::CreateContext(true);
#endif
      if (!status)
        {
          log_error("ChannelAccessInstructionHelper::HandleConnect('%s') - .. unable to attach to CA context", channel);
        }
    }

  if (status)
    {
      log_debug("ChannelAccessInstructionHelper::HandleConnect('%s') - Connect to variable ..", channel);
      _connected = ::ccs::HelperTools::ChannelAccess::ConnectVariable(channel, _channel);

      if (false == _connected)
        { // Might be related to the connection test included call above
          log_debug("ChannelAccessInstructionHelper::HandleConnect('%s') - .. give it more time", channel);
          (void)::ccs::HelperTools::SleepFor(_delay);
          _connected = ::ccs::HelperTools::ChannelAccess::IsConnected(_channel);
        }

      status = _connected;
    }
#ifdef LOG_DEBUG_ENABLE
  if (!status)
    {
      log_error("ChannelAccessInstructionHelper::HandleConnect('%s') - .. failure", channel);
    }
#endif
  return status;

}

bool ChannelAccessInstructionHelper::SetDelay (ccs::types::uint64 delay) { _delay = delay; return true; }
bool ChannelAccessInstructionHelper::SetDelay (const ccs::types::char8 * const delay) { return SetDelay(::ccs::HelperTools::ToInteger<ccs::types::uint64>(delay)); }

chid ChannelAccessInstructionHelper::GetChannel (void) const { return _channel; }

bool ChannelAccessInstructionHelper::VerifyValue (const ccs::types::AnyValue& value) const
{

  ::ccs::base::SharedReference<const ccs::types::AnyType> _type = value.GetType();

  bool status = static_cast<bool>(_type);

  if (status)
    { // ToDo - Should be scalar or scalar array .. not array of structures
      status = (::ccs::HelperTools::Is<ccs::types::ArrayType>(_type) ||
                ::ccs::HelperTools::Is<ccs::types::ScalarType>(_type));
    }

  return status;

}

bool ChannelAccessInstructionHelper::ReadChannel (ccs::types::AnyValue& value) const
{

  bool status = _connected;

  if (status)
    {
      log_debug("ChannelAccessInstructionHelper::ReadChannel - Access as type '%s' ..", value.GetType()->GetName());

      if (::ccs::HelperTools::Is<ccs::types::ScalarType>(value.GetType()))
        {
          status = ::ccs::HelperTools::ChannelAccess::ReadVariable(GetChannel(), ::ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), value.GetInstance());
        }
      else
        {
          ::ccs::base::SharedReference<const ccs::types::ArrayType> _type = value.GetType();
          status = ::ccs::HelperTools::ChannelAccess::ReadVariable(GetChannel(), ::ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), _type->GetMultiplicity(), value.GetInstance());
        }
    }

  return status;

}

bool ChannelAccessInstructionHelper::WriteChannel (const ccs::types::AnyValue& value) const
{

  bool status = _connected;

  if (status)
    {
      log_debug("ChannelAccessInstructionHelper::WriteChannel - Access as type '%s' ..", value.GetType()->GetName());

      if (::ccs::HelperTools::Is<ccs::types::ScalarType>(value.GetType()))
        {
          status = ::ccs::HelperTools::ChannelAccess::WriteVariable(GetChannel(), ::ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), value.GetInstance());
        }
      else
        {
          ::ccs::base::SharedReference<const ccs::types::ArrayType> _type = value.GetType();
          status = ::ccs::HelperTools::ChannelAccess::WriteVariable(GetChannel(), ::ccs::HelperTools::AnyTypeToCAScalar(value.GetType()), _type->GetMultiplicity(), value.GetInstance());
        }
    }

  return status;

}

bool ChannelAccessInstructionHelper::HandleDetach (void)
{

  log_debug("ChannelAccessInstructionHelper::HandleDetach - Method called for '%u'", _channel);

  bool status = _connected;

  if (!status)
    {
      log_warning("ChannelAccessInstructionHelper::HandleDetach - Channel not connected ..");
    }

  if (status)
    { // Detach from CA variable
      (void)::ccs::HelperTools::ChannelAccess::DetachVariable(_channel);
      _connected = false;
    }

  // Detach from CA context .. implicit destroy when necessary    
#ifdef ChannelAccessInstruction_Share_Context
  ::ccs::HelperTools::ChannelAccessClientContext::Detach();
#else
  ::ccs::HelperTools::ChannelAccess::ClearContext();
#endif
  return status;

}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
bool ChannelAccessFetchInstruction::SetupImpl (const Procedure& proc)
{

  log_debug("ChannelAccessFetchInstruction('%s')::SetupImpl - Method called ..", Instruction::GetName().c_str());

  bool status = (Instruction::HasAttribute("channel") && Instruction::HasAttribute("variable"));

  if (status)
    {
      log_debug("ChannelAccessFetchInstruction('%s')::SetupImpl - Method called with channel '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
      log_debug("ChannelAccessFetchInstruction('%s')::SetupImpl - .. using workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());

      // Verify if the named variable exists in the workspace ..
      status = (proc.VariableNames().end() != std::find(proc.VariableNames().begin(), proc.VariableNames().end(), Instruction::GetAttribute("variable").c_str()));
    }

  if (status)
    { // .. in order to access the expected datatype
      status = proc.GetVariableValue(Instruction::GetAttribute("variable"), _value);
    }

  if (status)
    {
      status = ChannelAccessInstructionHelper::VerifyValue(_value);
    }
#ifdef LOG_DEBUG_ENABLE
  if (!status)
    {
      log_error("ChannelAccessFetchInstruction('%s')::SetupImpl - .. failure", Instruction::GetName().c_str());
    }
#endif
  if (status && Instruction::HasAttribute("delay"))
    {
      status = ChannelAccessInstructionHelper::SetDelay(Instruction::GetAttribute("delay").c_str());
    }

  return status;

}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
ExecutionStatus ChannelAccessFetchInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  log_debug("ChannelAccessFetchInstruction('%s')::ExecuteSingleImpl - Method called ..", Instruction::GetName().c_str());
#ifdef ChannelAccessInstruction_Use_Mutex
  // MUTEX across multiple threads
  std::lock_guard<std::mutex> lock (_async_mutex);
#endif
  (void)ui;
  (void)ws;

  bool status = static_cast<bool>(_value.GetType());

  if (status)
    { // Attach to CA variable
      log_debug("ChannelAccessFetchInstruction('%s')::ExecuteSingleImpl - Connect ..", Instruction::GetName().c_str());
      status = ChannelAccessInstructionHelper::HandleConnect(Instruction::GetAttribute("channel").c_str());
    }

  if (status)
    {
      log_debug("ChannelAccessFetchInstruction('%s')::ExecuteSingleImpl - Read channel ..", Instruction::GetName().c_str());
      status = ChannelAccessInstructionHelper::ReadChannel(_value);
    }

  if (status)
    { // Write to workspace
      log_debug("ChannelAccessFetchInstruction('%s')::ExecuteSingleImpl - Update workspace ..", Instruction::GetName().c_str());
      status = ws->SetValue(Instruction::GetAttribute("variable"), _value);
    }
#ifdef LOG_DEBUG_ENABLE
  if (status && ws->GetValue(Instruction::GetAttribute("variable"), _value))
    {
      ccs::types::string buffer;
      _value.SerialiseInstance(buffer, ccs::types::MaxStringLength);
      log_debug("ChannelAccessFetchInstruction('%s')::ExecuteSingleImpl - .. variable has '%s' value in the workspace", Instruction::GetName().c_str(), buffer);
    }
#endif
  // Detach from CA variable
  log_debug("ChannelAccessFetchInstruction('%s')::ExecuteSingleImpl - Detach ..", Instruction::GetName().c_str());
  (void)ChannelAccessInstructionHelper::HandleDetach();

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

bool ChannelAccessWriteInstruction::VerifyAttributes (const Procedure& proc) const
{

  bool status = Instruction::HasAttribute("channel");

  if (status)
    {
      log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - Method called with channel '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
      status = ((Instruction::HasAttribute("variable") && (proc.VariableNames().end() != std::find(proc.VariableNames().begin(), proc.VariableNames().end(), Instruction::GetAttribute("variable").c_str()))) ||
                (Instruction::HasAttribute("datatype") && Instruction::HasAttribute("instance")));
    }

  return status;

}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
bool ChannelAccessWriteInstruction::SetupImpl (const Procedure& proc)
{

  log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - Method called ..", Instruction::GetName().c_str());

  bool status = VerifyAttributes(proc);

  if (status)
    {
      if (Instruction::HasAttribute("variable"))
        { // Read variable to allow verifying the type
          log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - .. using workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());
          status = proc.GetVariableValue(Instruction::GetAttribute("variable"), _value);
        }
      else
        {
          log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - .. using type '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("datatype").c_str());
          log_debug("ChannelAccessWriteInstruction('%s')::SetupImpl - .. and instance '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("instance").c_str());
          _value = ccs::types::AnyValue (Instruction::GetAttribute("datatype").c_str());
          status = _value.ParseInstance(Instruction::GetAttribute("instance").c_str());
        }
    }

  if (status)
    {
      status = ChannelAccessInstructionHelper::VerifyValue(_value);
    }
#ifdef LOG_DEBUG_ENABLE
  if (!status)
    {
      log_error("ChannelAccessWriteInstruction('%s')::SetupImpl - .. failure", Instruction::GetName().c_str());
    }
#endif
  if (status && Instruction::HasAttribute("delay"))
    {
      status = ChannelAccessInstructionHelper::SetDelay(Instruction::GetAttribute("delay").c_str());
    }

  return status;

}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
ExecutionStatus ChannelAccessWriteInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Method called ..", Instruction::GetName().c_str());
#ifdef ChannelAccessInstruction_Use_Mutex
  // MUTEX across multiple threads
  std::lock_guard<std::mutex> lock (_async_mutex);
#endif
  (void)ui;
  (void)ws;

  bool status = static_cast<bool>(_value.GetType());

  if (status)
    { // Attach to CA variable
      log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Connect ..", Instruction::GetName().c_str());
      status = ChannelAccessInstructionHelper::HandleConnect(Instruction::GetAttribute("channel").c_str());
    }

  if (status && Instruction::HasAttribute("variable"))
    { // Update from workspace variable that may have changed since setup
      log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Use workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());
      status = ws->GetValue(Instruction::GetAttribute("variable"), _value);
    }

  if (status)
    {
      log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Write ..", Instruction::GetName().c_str());
      status = ChannelAccessInstructionHelper::WriteChannel(_value);
    }

  // Detach from CA variable
  log_debug("ChannelAccessWriteInstruction('%s')::ExecuteSingleImpl - Detach ..", Instruction::GetName().c_str());
  (void)ChannelAccessInstructionHelper::HandleDetach();

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

ChannelAccessInstructionHelper::ChannelAccessInstructionHelper (void)
{
#ifdef ChannelAccessInstruction_Use_Mutex
  // MUTEX across multiple threads
  std::lock_guard<std::mutex> lock (_async_mutex);
#endif
#ifdef ChannelAccessInstruction_Share_Context
/  // Create CA context
  (void)::ccs::HelperTools::ChannelAccessClientContext::CreateAsNecessary();
#endif
}

ChannelAccessInstructionHelper::~ChannelAccessInstructionHelper (void)
{
#ifdef ChannelAccessInstruction_Use_Mutex
  // MUTEX across multiple threads
  std::lock_guard<std::mutex> lock (_async_mutex);
#endif
#ifdef ChannelAccessInstruction_Share_Context
  // Destroy CA context
  (void)::ccs::HelperTools::ChannelAccessClientContext::TerminateWhenAppropriate();
#endif
}

ChannelAccessFetchInstruction::ChannelAccessFetchInstruction (void) : Instruction(ChannelAccessFetchInstruction::Type), ChannelAccessInstructionHelper() {}
ChannelAccessFetchInstruction::~ChannelAccessFetchInstruction (void) {}

ChannelAccessWriteInstruction::ChannelAccessWriteInstruction (void) : Instruction(ChannelAccessWriteInstruction::Type), ChannelAccessInstructionHelper() {}
ChannelAccessWriteInstruction::~ChannelAccessWriteInstruction (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
