/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : Instruction node implementation
*
* Author        : Walter Van Herck (IO)
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

#include <algorithm> // std::find, etc.

#include <common/PVAccessClient.h>

#include <common/log-api.h> // Syslog wrapper routines

#include <Instruction.h>
#include <InstructionRegistry.h>
#include <Procedure.h>
#include <Workspace.h>

// Local header files

#include "ToInteger.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief PVAccessWriteInstruction class.
 * @detail Blocking instruction which establishes a PVAccess connection and updates the
 * channel with the value of a workspace variable. Mandatory attributes are 'channel' (PV name)
 * and 'variable' workspace variable name.
 */
class PVAccessWriteInstruction : public Instruction
{
  private:
    /**
     * @brief The PVAccess channel
     */
    ::ccs::base::PVAccessClient pva_client;

    /**
     * @brief See sup::sequencer::Instruction.
     */
    bool SetupImpl(const Procedure& proc) override;

    /**
     * @brief See sup::sequencer::Instruction.
     */
    ExecutionStatus ExecuteSingleImpl(UserInterface * ui, Workspace * ws) override;

  protected:

  public:
    PVAccessWriteInstruction();
    ~PVAccessWriteInstruction() override;

    /**
     * @brief Instruction name for InstructionRegistry.
     */
    static const std::string Type;
};

// Function declaration

// Global variables

const std::string PVAccessWriteInstruction::Type = "PVAccessWriteInstruction";
static bool _pvaccesswrite_initialised_flag = RegisterGlobalInstruction<PVAccessWriteInstruction>();

// Function definition

bool PVAccessWriteInstruction::SetupImpl(const Procedure &proc)
{

  log_debug("PVAccessWriteInstruction('%s')::SetupImpl - Method called ..", GetName().c_str());

  bool status = (HasAttribute("channel") && HasAttribute("variable"));

  if (status)
  {
    log_debug("PVAccessWriteInstruction('%s')::SetupImpl - .. with channel '%s'", GetName().c_str(), GetAttribute("channel").c_str());
    log_debug("PVAccessWriteInstruction('%s')::SetupImpl - .. using workspace variable '%s'", GetName().c_str(), GetAttribute("variable").c_str());

    // Verify the variable exists in the workspace
    status = (proc.VariableNames().end() != std::find(proc.VariableNames().begin(), proc.VariableNames().end(), GetAttribute("variable").c_str()));
  }

  if (status)
  { // Do not try to add the same channel twice
    status = !pva_client.IsValid(GetAttribute("channel").c_str());
  }

  if (status)
  {
    status = pva_client.AddVariable(GetAttribute("channel").c_str(), ::ccs::types::OutputVariable);
  }

  if (status)
  {
    status = pva_client.Launch();
  }

  return status;
}

ExecutionStatus PVAccessWriteInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  log_debug("PVAccessWriteInstruction('%s')::ExecuteSingleImpl - Method called ..", GetName().c_str());

  (void)ui;
  (void)ws;

  bool status = HasAttribute("channel");

  if (status)
  {
    status = pva_client.IsConnected(GetAttribute("channel").c_str());
  }

  ccs::types::AnyValue _value;

  if (status)
  {
    status = ws->GetValue(GetAttribute("variable"), _value);
  }

  if (status)
  { // Write to channel
    log_debug("PVAccessWriteInstruction('%s')::ExecuteSingleImpl - .. write variable '%s' to PVAccess channel",
              GetName().c_str(), GetAttribute("variable").c_str());
    status = pva_client.SetVariable(GetAttribute("channel").c_str(), _value);
  }

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);
}

PVAccessWriteInstruction::PVAccessWriteInstruction()
  : Instruction(PVAccessWriteInstruction::Type)
{}

PVAccessWriteInstruction::~PVAccessWriteInstruction() {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
