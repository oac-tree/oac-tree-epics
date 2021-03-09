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

#include <Instruction.h>
#include <InstructionRegistry.h>

#include <Workspace.h>
#include <Variable.h>

// Local header files

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

/**
 * @brief Instruction copying named variable from/to workspace.
 */

class CopyInstruction : public Instruction
{

  private:

    /**
     * @brief See sup::sequencer::Instruction.
     * @details Copy variables identified with 'input' and 'output' attributes.
     */

    ExecutionStatus ExecuteSingleImpl (UserInterface * ui, Workspace * ws) override;

  protected:

  public:

    /**
     * @brief Constructor.
     */

    CopyInstruction (void);

    /**
     * @brief Destructor.
     */

    ~CopyInstruction (void) override;

    /**
     * @brief Class name for InstructionRegistry.
     */

    static const std::string Type;

};

// Function declaration

// Global variables

const std::string CopyInstruction::Type = "CopyInstruction";
static bool _copyinstruction_initialised_flag = RegisterGlobalInstruction<CopyInstruction>();

// Function definition

ExecutionStatus CopyInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  (void)ui;
  (void)ws;

  bool status = (Instruction::HasAttribute("input") && Instruction::HasAttribute("output"));

  ccs::types::AnyValue _value; // Placeholder

  if (status)
    { // Read from workspace
      status = ws->GetValue(Instruction::GetAttribute("input"), _value);
    }

  if (status)
    { // Write to workspace
      status = ws->SetValue(Instruction::GetAttribute("output"), _value);
    }

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

CopyInstruction::CopyInstruction (void) : Instruction("CopyInstruction") {}
CopyInstruction::~CopyInstruction (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
