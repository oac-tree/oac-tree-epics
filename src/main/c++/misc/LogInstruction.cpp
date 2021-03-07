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

#include <common/BasicTypes.h> // Misc. type definition

#include <common/log-api.h> // Syslog wrapper routines

#include <common/AnyValue.h>
#include <common/AnyValueHelper.h>

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

static inline ccs::log::Severity_t GetSeverity (const std::string& severity);

// Global variables

const std::string LogInstruction::Type = "LogTrace";
static bool _log_initialised_flag = RegisterGlobalInstruction<LogInstruction>();

// Function definition

static inline ccs::log::Severity_t GetSeverity (const std::string& severity)
{

  ccs::log::Severity_t ret = LOG_INFO;

  if (severity == "debug")
    {
      ret = LOG_DEBUG;
    }

  if (severity == "notice")
    {
      ret = LOG_NOTICE;
    }

  return ret;

}

ExecutionStatus LogInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  (void)ui;
  (void)ws;

  bool status = (Instruction::HasAttribute("message") || Instruction::HasAttribute("input"));

  if (status && (false == Instruction::HasAttribute("level")))
    {
      status = Instruction::AddAttribute("level", "info");
    }

  if (Instruction::HasAttribute("message"))
    {
      ccs::log::Message(GetSeverity(Instruction::GetAttribute("level")), LOG_ALTERN_SRC, Instruction::GetAttribute("message").c_str());
    }

  if (Instruction::HasAttribute("input"))
    { // Read from workspace
      ccs::types::AnyValue _value;
      status = ws->GetValue(Instruction::GetAttribute("input"), _value);

      if (status)
        {
          ccs::types::char8 buffer [2048];
          (void)ccs::HelperTools::SerialiseToJSONStream(&_value, buffer, 2048u);
          ccs::log::Message(GetSeverity(Instruction::GetAttribute("level")), LOG_ALTERN_SRC, buffer);
        }
    }

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

LogInstruction::LogInstruction (void) : Instruction(Type) {}
LogInstruction::~LogInstruction (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
