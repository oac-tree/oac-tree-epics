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

#include <algorithm> // std::find, etc.

#include <common/BasicTypes.h> // Misc. type definition
#include <common/StringTools.h> // Misc. helper functions
#include <common/TimeTools.h> // Misc. helper functions

#include <common/log-api.h> // Syslog wrapper routines

#include <common/RPCClient.h> // Plug-in managed through dynamic linking .. see Makefile

#include <Instruction.h>
#include <InstructionRegistry.h>

#include <Workspace.h>

// Local header files

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief Xxx
 */

class RPCClientInstruction : public Instruction
{

  private:

    ccs::types::AnyValue _request;

    /**
     * @brief Verify and handle attributes.
     */

    virtual bool SetupImpl (Workspace * ws);

    /**
     * @brief See sup::sequencer::Instruction.
     */

    ExecutionStatus ExecuteSingleImpl (UserInterface * ui, Workspace * ws) override;

  protected:

  public:

    /**
     * @brief Constructor.
     */

    RPCClientInstruction (void);

    /**
     * @brief Destructor.
     */

    ~RPCClientInstruction (void) override;

    /**
     * @brief Class name for InstructionRegistry.
     */

    static const std::string Type;

};

// Function declaration

// Global variables

const std::string RPCClientInstruction::Type = "RPCClientInstruction";
static bool _rpcclient_initialised_flag = RegisterGlobalInstruction<RPCClientInstruction>();

// Function definition

bool RPCClientInstruction::SetupImpl (Workspace * ws)
{

  log_debug("RPCClientInstruction('%s')::SetupImpl - Method called ..", Instruction::GetName().c_str());

  bool status = Instruction::HasAttribute("service");

  if (status)
    {
      log_debug("RPCClientInstruction::SetupImpl('%s') - Method called with service '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("service").c_str());
      status = ((Instruction::HasAttribute("request") && (ws->VariableNames().end() != std::find(ws->VariableNames().begin(), ws->VariableNames().end(), Instruction::GetAttribute("request").c_str()))) ||
                (Instruction::HasAttribute("datatype") && Instruction::HasAttribute("instance")));
    }

  if (status)
    {
      if (Instruction::HasAttribute("request"))
        {
          log_debug("RPCClientInstruction::SetupImpl('%s') - .. using workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("request").c_str());
          status = ws->GetValue(Instruction::GetAttribute("request"), _request);
        }
      else
        {
          log_debug("RPCClientInstruction::SetupImpl('%s') - .. using type '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("datatype").c_str());
          log_debug("RPCClientInstruction::SetupImpl('%s') - .. and instance '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("instance").c_str());
          _request = ccs::types::AnyValue (Instruction::GetAttribute("datatype").c_str());
          status = _request.ParseInstance(Instruction::GetAttribute("instance").c_str());
        }
    }

  if (status)
    {
      status = static_cast<bool>(_request.GetType());
    }

  return status;

}

ExecutionStatus RPCClientInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  log_debug("RPCClientInstruction::ExecuteSingleImpl('%s') - Method called ..", Instruction::GetName().c_str());

  (void)ui;
  (void)ws;

  bool status = SetupImpl(ws);
#ifdef LOG_DEBUG_ENABLE
  if (status)
    {
      ccs::types::char8 buffer [1024];

      log_debug("RPCClientInstruction::ExecuteSingleImpl('%s') - Using request ..", Instruction::GetName().c_str());
      _request.SerialiseType(buffer, 1024u);
      log_debug("RPCClientInstruction::ExecuteSingleImpl('%s') - .. type '%s'", Instruction::GetName().c_str(), buffer);
      _request.SerialiseInstance(buffer, 1024u);
      log_debug("RPCClientInstruction::ExecuteSingleImpl('%s') - .. instance '%s'", Instruction::GetName().c_str(), buffer);
    }
#endif
  ccs::types::AnyValue reply; // Placeholder

  if (status)
    { // Create RPC client
      ccs::base::RPCClient client (Instruction::GetAttribute("service").c_str());

      log_debug("RPCClientInstruction::ExecuteSingleImpl('%s') - Launch ..", Instruction::GetName().c_str());
      status = client.Launch();

      if (status)
        {
          (void)::ccs::HelperTools::SleepFor(100000000ul);
          status = client.IsConnected();
        }

      if (status)
        {
          log_debug("RPCClientInstruction::ExecuteSingleImpl('%s') - Send request ..", Instruction::GetName().c_str());
          reply = client.SendRequest(_request);
          status = static_cast<bool>(reply.GetType());
        }
    }

  if (status && ::ccs::HelperTools::HasAttribute(&reply, "status"))
    {
      status = ::ccs::HelperTools::GetAttributeValue<bool>(&reply, "status");
    }

  if (status && Instruction::HasAttribute("reply"))
    {
      //status = ws->SetValue(Instruction::GetAttribute("reply"), reply);
      (void)ws->SetValue(Instruction::GetAttribute("reply"), reply); // Does LocalVariable need to know the type beforehand ?
    }
#ifdef LOG_DEBUG_ENABLE
  if (status)
    {
      ccs::types::char8 buffer [1024];

      log_debug("RPCClientInstruction::ExecuteSingleImpl('%s') - Received reply ..", Instruction::GetName().c_str());
      reply.SerialiseType(buffer, 1024u);
      log_debug("RPCClientInstruction::ExecuteSingleImpl('%s') - .. type '%s'", Instruction::GetName().c_str(), buffer);
      reply.SerialiseInstance(buffer, 1024u);
      log_debug("RPCClientInstruction::ExecuteSingleImpl('%s') - .. instance '%s'", Instruction::GetName().c_str(), buffer);
    }
#endif
  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

RPCClientInstruction::RPCClientInstruction (void) : Instruction(RPCClientInstruction::Type) {}
RPCClientInstruction::~RPCClientInstruction (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
