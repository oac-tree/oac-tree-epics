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

#include <common/AnyTypeDatabase.h>

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
 * @brief Registers datatypes to the GlobalTypeDatabase.
 */

class RegisterTypeInstruction : public Instruction
{

  private:

    /**
     * @brief See sup::sequencer::Instruction.
     * @details Registers User-defined 'datatype' to the process GlobalTypeDatabase.
     */

    ExecutionStatus ExecuteSingleImpl (UserInterface * ui, Workspace * ws) override;

  protected:

  public:

    /**
     * @brief Constructor.
     */

    RegisterTypeInstruction (void);

    /**
     * @brief Destructor.
     */

    ~RegisterTypeInstruction (void) override;

    /**
     * @brief Class name for InstructionRegistry.
     */

    static const std::string Type;

};

// Function declaration

// Global variables

const std::string RegisterTypeInstruction::Type = "RegisterType";
static bool _register_initialised_flag = RegisterGlobalInstruction<RegisterTypeInstruction>();

// Function definition

ExecutionStatus RegisterTypeInstruction::ExecuteSingleImpl (UserInterface * ui, Workspace * ws)
{

  (void)ui;
  (void)ws;

  bool status = Instruction::HasAttribute("datatype");

  if (status)
    {
      log_info("RegisterTypeInstruction('%s')::ExecuteSingleImpl - Method called with datatype '%s' ..", Instruction::GetName().c_str(), Instruction::GetAttribute("datatype").c_str());
      status = ::ccs::base::GlobalTypeDatabase::Register(Instruction::GetAttribute("datatype").c_str());
    }

  return (status ? ExecutionStatus::SUCCESS : ExecutionStatus::FAILURE);

}

RegisterTypeInstruction::RegisterTypeInstruction (void) : Instruction(Type) {}
RegisterTypeInstruction::~RegisterTypeInstruction (void) {}

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
