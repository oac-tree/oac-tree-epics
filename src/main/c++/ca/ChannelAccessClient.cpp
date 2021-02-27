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

#include <new> // std::nothrow, etc.
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
#include "Variable.h"

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
 */

class BlockingCAFetchNode : public Instruction
{

  private:

    /**
     * @brief CA channel identifier attribute.
     */

    chid _channel;

    /**
     * @brief Workspace variable copy.
     */

    ccs::types::AnyValue _value;

    /**
     * @brief Xxx
     */

    ExecutionStatus ExecuteSingleImpl (UserInterface * ui, Workspace * ws) override;

  protected:

  public:

    /**
     * @brief Constructor.
     */

    BlockingCAFetchNode (void);

    /**
     * @brief Destructor.
     */

    ~BlockingCAFetchNode (void) override;

};

class BlockingCAWriteNode : public Instruction
{

  private:

    /**
     * @brief CA channel identifier attribute.
     */

    chid _channel;

    /**
     * @brief Workspace variable copy.
     */

    ccs::types::AnyValue _value;

    /**
     * @brief Xxx
     */

    ExecutionStatus ExecuteSingleImpl (UserInterface * ui, Workspace * ws) override;

  protected:

  public:

    /**
     * @brief Constructor.
     */

    BlockingCAWriteNode (void);

    /**
     * @brief Destructor.
     */

    ~BlockingCAWriteNode (void) override;

};

// Function declaration

bool RegisterInstruction_ChannelAccessClient (void);

// Global variables

static bool global_caclient_initialised_flag = RegisterInstruction_ChannelAccessClient();

// Function definition

bool RegisterInstruction_ChannelAccessClient (void)
{

  { // CAFetch registration
    auto constructor = []() { return static_cast<Instruction*>(new BlockingCAFetchNode ()); };
    GlobalInstructionRegistry().RegisterInstruction("BlockingCAFetchNode", constructor);
  }

  { // CAWrite registration
    auto constructor = []() { return static_cast<Instruction*>(new BlockingCAWriteNode ()); };
    GlobalInstructionRegistry().RegisterInstruction("BlockingCAWriteNode", constructor);
  }

  return true;

}

ExecutionStatus BlockingCAFetchNode::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  (void)ui;
  (void)ws;

  bool status = (Instruction::HasAttribute("channel") && Instruction::HasAttribute("variable"));

  if (status)
    {
      log_info("BlockingCAFetchNode::ExecuteSingleImpl('%s') - Method called with channel '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
      log_info("BlockingCAFetchNode::ExecuteSingleImpl('%s') - .. using workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());

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

  if (status)
    { // Attach to CA context .. implicit create if necessary
      status = ::ccs::HelperTools::ChannelAccessClientContext::Attach();
    }

  if (status)
    {
      log_info("BlockingCAFetchNode::ExecuteSingleImpl('%s') - Connect to variable '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
      status = ccs::HelperTools::ChannelAccess::ConnectVariable(GetAttribute("channel").c_str(), _channel);
    }

  if (status && ::ccs::HelperTools::ChannelAccess::IsConnected(_channel))
    {
      log_info("BlockingCAFetchNode::ExecuteSingleImpl('%s') - Fetch as type '%s' ..", Instruction::GetName().c_str(), _value.GetType()->GetName());
      status = ::ccs::HelperTools::ChannelAccess::ReadVariable(_channel, ::ccs::HelperTools::AnyTypeToCAScalar(_value.GetType()), _value.GetInstance());
    }

  if (status)
    { // Write to workspace
      status = ws->SetValue(GetAttribute("variable"), _value);
    }

  if (status && ws->GetValue(GetAttribute("variable"), _value))
    {
      ccs::types::string buffer;
      _value.SerialiseInstance(buffer, ccs::types::MaxStringLength);
      log_info("BlockingCAFetchNode::ExecuteSingleImpl('%s') - .. variable has '%s' value in the workspace", Instruction::GetName().c_str(), buffer);
    }

  // Detach from CA context .. implicit destroy when necessary
  ::ccs::HelperTools::ChannelAccessClientContext::Detach();

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

ExecutionStatus BlockingCAWriteNode::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  (void)ui;
  (void)ws;

  bool status = Instruction::HasAttribute("channel");

  if (status)
    {
      log_info("BlockingCAWriteNode::ExecuteSingleImpl('%s') - Method called with channel '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
      status = ((Instruction::HasAttribute("variable") && (ws->VariableNames().end() != std::find(ws->VariableNames().begin(), ws->VariableNames().end(), Instruction::GetAttribute("variable").c_str()))) ||
                (Instruction::HasAttribute("datatype") && Instruction::HasAttribute("instance")));
    }

  if (status)
    {
      if (Instruction::HasAttribute("variable"))
        {
          log_info("BlockingCAWriteNode::ExecuteSingleImpl('%s') - .. using workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());
          status = ws->GetValue(GetAttribute("variable"), _value);
        }
      else
        {
          log_info("BlockingCAWriteNode::ExecuteSingleImpl('%s') - .. using type '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("datatype").c_str());
          log_info("BlockingCAWriteNode::ExecuteSingleImpl('%s') - .. and instance '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("instance").c_str());
          _value = ccs::types::AnyValue (Instruction::GetAttribute("datatype").c_str());
          status = _value.ParseInstance(Instruction::GetAttribute("instance").c_str());
        }
    }

  if (status)
    { // Attach to CA context .. implicit create if necessary
      status = ::ccs::HelperTools::ChannelAccessClientContext::Attach();
    }

  if (status)
    {
      log_info("BlockingCAWriteNode::ExecuteSingleImpl('%s') - Connect to variable '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
      status = ccs::HelperTools::ChannelAccess::ConnectVariable(Instruction::GetAttribute("channel").c_str(), _channel);
    }

  if (status && ::ccs::HelperTools::ChannelAccess::IsConnected(_channel))
    {
      log_info("BlockingCAWriteNode::ExecuteSingleImpl('%s') - Write as type '%s' ..", Instruction::GetName().c_str(), _value.GetType()->GetName());
      status = ::ccs::HelperTools::ChannelAccess::WriteVariable(_channel, ccs::HelperTools::AnyTypeToCAScalar(_value.GetType()), _value.GetInstance());
    }

  // Detach from CA context .. implicit destroy when necessary
  ::ccs::HelperTools::ChannelAccessClientContext::Detach();

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

BlockingCAFetchNode::BlockingCAFetchNode (void) : Instruction("BlockingCAFetchNode") {}
BlockingCAFetchNode::~BlockingCAFetchNode (void) {}

BlockingCAWriteNode::BlockingCAWriteNode (void) : Instruction("BlockingCAWriteNode") {}
BlockingCAWriteNode::~BlockingCAWriteNode (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
