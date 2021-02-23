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

#include <Instruction.h>
#include <InstructionRegistry.h>

// Local header files

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief Obvious.
 */

class LogInstruction : public Instruction
{

  private:

    /**
     * @brief Logging trace using 'message' attribute value.
     */

    ExecutionStatus ExecuteSingleImpl (UserInterface * ui, Workspace * ws) override;

  protected:

  public:

    /**
     * @brief Constructor.
     */

    LogInstruction (void);

    /**
     * @brief Destructor.
     */

    ~LogInstruction (void) override;

    /**
     * @brief Class name for InstructionRegistry.
     */

    static const std::string Type;

};

// Function declaration

// Global variables

const std::string LogInstruction::Type = "LogTrace";
static bool _log_initialised_flag = RegisterGlobalInstruction<LogInstruction>();

// Function definition

ExecutionStatus LogInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  (void)ui;
  (void)ws;

  bool status = Instruction::HasAttribute("message");

  if (status)
    {
      log_info(Instruction::GetAttribute("message").c_str());
    }

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

LogInstruction::LogInstruction (void) : Instruction(Type) {}
LogInstruction::~LogInstruction (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
