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

#include <new> // std::nothrow, etc.
#include <mutex> // std::mutex, etc.
#include <algorithm> // std::find, etc.

#include <common/BasicTypes.h> // Misc. type definition
#include <common/StringTools.h> // Misc. helper functions
#include <common/TimeTools.h> // Misc. helper functions

#define LOG_DEBUG_ENABLE
#include <common/log-api.h> // Syslog wrapper routines

#include <common/PVMonitor.h> // Plug-in managed through dynamic linking .. see Makefile

// Local header files

#include "PVMonitorCache.h"

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
 * @brief PVMonitorInstruction class.
 * @detail Blocking instruction which establishes a PVAccess connection and updates a
 * workspace variable based on monitor received. Mandatory attributes are 'channel' (PV name)
 * and 'variable' workspace variable name. The 'timeout' attribute is optional and defaulted
 * to 5s.
 */

class PVMonitorInstruction : public Instruction, public PVMonitorCache
{

  private:

    /**
     * @brief Timeout parameter.
     */

    ccs::types::uint64 _timeout = 5000000000ul;

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

    PVMonitorInstruction (void);

    /**
     * @brief Destructor.
     */

    ~PVMonitorInstruction (void) override;

    /**
     * @brief Class name for InstructionRegistry.
     */

    static const std::string Type;

};

// Function declaration

template <typename Type> inline Type ToInteger (const ccs::types::char8 * const buffer);

// Global variables

const std::string PVMonitorInstruction::Type = "PVMonitorInstruction";
static bool _pvmonitor_initialised_flag = RegisterGlobalInstruction<PVMonitorInstruction>();

// Function definition

template <> inline ccs::types::uint64 ToInteger<ccs::types::uint64> (const ccs::types::char8 * const buffer)
{

  ccs::types::uint64 ret = 0ul;

  bool status = (ccs::HelperTools::IsIntegerString(buffer) == true);

  if (status)
    {
      ccs::types::char8* p = NULL_PTR_CAST(ccs::types::char8*);
      ret = static_cast<ccs::types::uint64>(strtoul(buffer, &p, 10));
    }

  return ret;

}

bool PVMonitorInstruction::SetupImpl (Workspace * ws)
{

  log_debug("PVMonitorInstruction('%s')::SetupImpl - Method called ..", Instruction::GetName().c_str());

  bool status = (HasAttribute("channel") && HasAttribute("variable") && (false == PVMonitorCache::IsInitialised()));

  if (status)
    {
      log_debug("PVMonitorInstruction('%s')::SetupImpl - .. with channel '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("channel").c_str());
      log_debug("PVMonitorInstruction('%s')::SetupImpl - .. using workspace variable '%s'", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());

      // Verify the variable exists in the workspace
      status = (ws->VariableNames().end() != std::find(ws->VariableNames().begin(), ws->VariableNames().end(), Instruction::GetAttribute("variable").c_str()));
    }

  if (status && Instruction::HasAttribute("timeout"))
    { // Move to HelperTools namespace
      _timeout = ToInteger<ccs::types::uint64>(Instruction::GetAttribute("timeout").c_str());
    }

  if (status)
    { // Instantiate implementation
      status = PVMonitorCache::SetChannel(Instruction::GetAttribute("channel").c_str());
    }

  return status;

}

ExecutionStatus PVMonitorInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  log_debug("PVMonitorInstruction('%s')::ExecuteSingleImpl - Method called ..", Instruction::GetName().c_str());

  (void)ui;
  (void)ws;

  bool status = SetupImpl(ws);

  while (status && (0ul < _timeout) && (false == PVMonitorCache::IsInitialised()))
    { // Wait at most timeout for a monitor to be received
      (void)ccs::HelperTools::SleepFor(100000ul);

      if (100000ul < _timeout)
        {
          _timeout -= 100000ul;
        }
      else
        {
          _timeout = 0ul;
        }
    }

  if (status)
    {
      status = PVMonitorCache::IsInitialised(); // Has received monitor and valid value
    }

  ccs::types::AnyValue _value; // Placeholder

  if (status)
    {
      status = PVMonitorCache::GetValue(_value);
    }

  if (status)
    { // Write to workspace
      log_debug("PVMonitorInstruction('%s')::ExecuteSingleImpl - .. update '%s' workspace variable", Instruction::GetName().c_str(), Instruction::GetAttribute("variable").c_str());
      status = ws->SetValue(GetAttribute("variable"), _value);
    }

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

PVMonitorInstruction::PVMonitorInstruction (void) : Instruction(PVMonitorInstruction::Type), PVMonitorCache() {}
PVMonitorInstruction::~PVMonitorInstruction (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
