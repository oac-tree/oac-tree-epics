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

#include <cstdlib> // std::system, etc.

#include <common/BasicTypes.h> // Misc. type definition
//#include <common/SysTools.h> // Misc. helper functions

#include <common/log-api.h> // Syslog wrapper routines

// Local header files

#include "Instruction.h"
#include "InstructionRegistry.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief Obvious.
 */

class SystemCallInstruction : public Instruction
{

  private:

    /**
     * @brief System call using 'command' attribute value.
     */

    ExecutionStatus ExecuteSingleImpl (UserInterface * ui, Workspace * ws) override;

  protected:

  public:

    /**
     * @brief Constructor.
     */

    SystemCallInstruction (void);

    /**
     * @brief Destructor.
     */

    ~SystemCallInstruction (void) override;

    /**
     * @brief Class name for InstructionRegistry.
     */

    static const std::string Type;

};

// Function declaration

// Global variables

const std::string SystemCallInstruction::Type = "SystemCall";
static bool _systemcall_initialised_flag = RegisterGlobalInstruction<SystemCallInstruction>();

// Function definition

ExecutionStatus SystemCallInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  (void)ui;
  (void)ws;

  bool status = Instruction::HasAttribute("command");

  if (status)
    {
      log_info("SystemCallInstruction('%s')::ExecuteSingleImpl - Method called with system command '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("command").c_str());
      status = (0 == std::system(Instruction::GetAttribute("command").c_str()));
    }

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

SystemCallInstruction::SystemCallInstruction (void) : Instruction(Type) {}
SystemCallInstruction::~SystemCallInstruction (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
