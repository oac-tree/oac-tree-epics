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

#include <common/BasicTypes.h> // Misc. type definition
#include <common/StringTools.h> // Misc. helper functions
#include <common/TimeTools.h> // Misc. helper functions

#include <common/log-api.h> // Syslog wrapper routines

#include <common/AnyValue.h>
#include <common/AnyValueHelper.h>

//#include <common/ChannelAccessClientContext.h>
#include <common/ChannelAccessHelper.h> // CA helper routines
#include <common/AnyTypeToCA.h> // CA helper routines .. type conversion

// Local header files

#include "ChannelAccessClientContext.h"

#include "ExecutionStatus.h"

#include "Instruction.h"
#include "InstructionRegistry.h"

#include "Workspace.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief Xxx
 */

class ChannelAccessInstructionHelper
{

  private:

    /**
     * @brief CA channel identifier attribute.
     */

    chid _channel;
    bool _connected = false;

  protected:

  public:

    bool HandleConnect (const ccs::types::char8 * const channel, ccs::types::uint64 delay = 100000000ul);
    bool HandleDetach (void);
    chid GetChannel (void) const;

    /**
     * @brief Constructor.
     */

    ChannelAccessInstructionHelper (void);

    /**
     * @brief Destructor.
     */

    virtual ~ChannelAccessInstructionHelper (void);

};

class ChannelAccessFetchInstruction : public Instruction, public ChannelAccessInstructionHelper
{

  private:

    /**
     * @brief Workspace variable copy.
     */

    ccs::types::AnyValue _value;

    /**
     * @brief Verify and handle attributes.
     */

    bool Setup (Workspace * ws);

    /**
     * @brief See sup::sequencer::Instruction.
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

class ChannelAccessWriteInstruction : public Instruction, public ChannelAccessInstructionHelper
{

  private:

    /**
     * @brief Workspace variable copy.
     */

    ccs::types::AnyValue _value;

    /**
     * @brief Verify and handle attributes.
     */

    bool Setup (Workspace * ws);

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

// Function definition

bool ChannelAccessInstructionHelper::HandleConnect (const ccs::types::char8 * const channel, ccs::types::uint64 delay)
{

  bool status = (false == _connected);

  if (status)
    { // Attach to CA context .. implicit create if necessary
      status = ::ccs::HelperTools::ChannelAccessClientContext::Attach();
    }

  if (status)
    {
      log_debug("ChannelAccessInstructionHelper::HandleConnect - Connect to variable '%s' ..", channel);
      //status = ::ccs::HelperTools::ChannelAccess::ConnectVariable(channel, _channel);
      (void)::ccs::HelperTools::ChannelAccess::ConnectVariable(channel, _channel);
      (void)::ccs::HelperTools::SleepFor(delay);
      _connected = ::ccs::HelperTools::ChannelAccess::IsConnected(_channel);
      status = _connected;
    }

  return status;

}

chid ChannelAccessInstructionHelper::GetChannel (void) const { return _channel; }

bool ChannelAccessInstructionHelper::HandleDetach (void)
{

  bool status = _connected;

  if (status)
    { // Detach from CA variable
      (void)::ccs::HelperTools::ChannelAccess::DetachVariable(_channel);
      _connected = false;
    }

  // Detach from CA context .. implicit destroy when necessary    
  ::ccs::HelperTools::ChannelAccessClientContext::Detach();

  return status;

}

bool ChannelAccessFetchInstruction::Setup (Workspace * ws)
{

  bool status = (Instruction::HasAttribute("channel") && Instruction::HasAttribute("variable"));

  if (status)
    {
      log_debug("ChannelAccessFetchInstruction::Setup('%s') - Method called with channel '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
      log_debug("ChannelAccessFetchInstruction::Setup('%s') - .. using workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());

      // Verify if the named variable exists in the workspace ..
      status = (ws->VariableNames().end() != std::find(ws->VariableNames().begin(), ws->VariableNames().end(), Instruction::GetAttribute("variable").c_str()));
    }

  if (status)
    { // .. in order to access the expected datatype
      status = ws->GetValue(Instruction::GetAttribute("variable"), _value);
    }

  if (status)
    {
      status = static_cast<bool>(_value.GetType());
    }

  return status;

}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
ExecutionStatus ChannelAccessFetchInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  (void)ui;
  (void)ws;

  bool status = Setup(ws);

  if (status)
    { // Attach to CA variable
      status = ChannelAccessInstructionHelper::HandleConnect(Instruction::GetAttribute("channel").c_str());
    }

  if (status)
    {
      log_debug("ChannelAccessFetchInstruction::ExecuteSingleImpl('%s') - Fetch as type '%s' ..", Instruction::GetName().c_str(), _value.GetType()->GetName());
      status = ::ccs::HelperTools::ChannelAccess::ReadVariable(ChannelAccessInstructionHelper::GetChannel(), ::ccs::HelperTools::AnyTypeToCAScalar(_value.GetType()), _value.GetInstance());
    }

  if (status)
    { // Write to workspace
      status = ws->SetValue(Instruction::GetAttribute("variable"), _value);
    }
#ifdef LOG_DEBUG_ENABLE
  if (status && ws->GetValue(Instruction::GetAttribute("variable"), _value))
    {
      ccs::types::string buffer;
      _value.SerialiseInstance(buffer, ccs::types::MaxStringLength);
      log_debug("ChannelAccessFetchInstruction::ExecuteSingleImpl('%s') - .. variable has '%s' value in the workspace", Instruction::GetName().c_str(), buffer);
    }
#endif
  if (status)
    { // Detach from CA variable
      (void)ChannelAccessInstructionHelper::HandleDetach();
    }

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

bool ChannelAccessWriteInstruction::Setup (Workspace * ws)
{

  bool status = Instruction::HasAttribute("channel");

  if (status)
    {
      log_debug("ChannelAccessWriteInstruction::Setup('%s') - Method called with channel '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
      status = ((Instruction::HasAttribute("variable") && (ws->VariableNames().end() != std::find(ws->VariableNames().begin(), ws->VariableNames().end(), Instruction::GetAttribute("variable").c_str()))) ||
                (Instruction::HasAttribute("datatype") && Instruction::HasAttribute("instance")));
    }

  if (status)
    {
      if (Instruction::HasAttribute("variable"))
        {
          log_debug("ChannelAccessWriteInstruction::Setup('%s') - .. using workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());
          status = ws->GetValue(Instruction::GetAttribute("variable"), _value);
        }
      else
        {
          log_debug("ChannelAccessWriteInstruction::Setup('%s') - .. using type '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("datatype").c_str());
          log_debug("ChannelAccessWriteInstruction::Setup('%s') - .. and instance '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("instance").c_str());
          _value = ccs::types::AnyValue (Instruction::GetAttribute("datatype").c_str());
          status = _value.ParseInstance(Instruction::GetAttribute("instance").c_str());
        }
    }

  if (status)
    {
      status = static_cast<bool>(_value.GetType());
    }

  return status;

}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
ExecutionStatus ChannelAccessWriteInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  (void)ui;
  (void)ws;

  bool status = Setup(ws);

  if (status)
    { // Attach to CA variable
      status = ChannelAccessInstructionHelper::HandleConnect(Instruction::GetAttribute("channel").c_str());
    }

  if (status)
    {
      log_debug("ChannelAccessWriteInstruction::ExecuteSingleImpl('%s') - Write as type '%s' ..", Instruction::GetName().c_str(), _value.GetType()->GetName());
      status = ::ccs::HelperTools::ChannelAccess::WriteVariable(ChannelAccessInstructionHelper::GetChannel(), ccs::HelperTools::AnyTypeToCAScalar(_value.GetType()), _value.GetInstance());
    }

  if (status)
    { // Detach from CA variable
      (void)ChannelAccessInstructionHelper::HandleDetach();
    }

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

ChannelAccessInstructionHelper::ChannelAccessInstructionHelper (void)
{
  // Create CA context
  (void)::ccs::HelperTools::ChannelAccessClientContext::CreateAsNecessary();

}

ChannelAccessInstructionHelper::~ChannelAccessInstructionHelper (void)
{
  // Destroy CA context
  (void)::ccs::HelperTools::ChannelAccessClientContext::TerminateWhenAppropriate();

}

ChannelAccessFetchInstruction::ChannelAccessFetchInstruction (void) : Instruction(ChannelAccessFetchInstruction::Type), ChannelAccessInstructionHelper() {}
ChannelAccessFetchInstruction::~ChannelAccessFetchInstruction (void) {}

ChannelAccessWriteInstruction::ChannelAccessWriteInstruction (void) : Instruction(ChannelAccessWriteInstruction::Type), ChannelAccessInstructionHelper() {}
ChannelAccessWriteInstruction::~ChannelAccessWriteInstruction (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
