/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Walter Van Herck
 *
 * Copyright (c) : 2010-2022 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_READ_INSTRUCTION_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_READ_INSTRUCTION_H_

#include <sup/sequencer/instruction.h>

#include <memory>

namespace sup
{
namespace dto
{
class AnyValue;
}  // namespace dto

namespace sequencer
{
/**
 * @brief PvAccessReadInstruction class.
 * @details Blocking instruction which establishes a PvAccess connection and reads the
 * channel's value to a workspace variable. Mandatory attributes are 'channel' (PV name) and
 * 'output' (variable name to write to).
 * The optional 'timeout' attribute causes the instruction
 * to wait for connection with the specified timeout first and fails if connection was still not
 * established.
 * @code
     <Sequence>
       <PvAccessRead name="read-pv"
         channel="EPICS::PVA::CHANNEL::BOOLEAN"
         output="boolean"/>
     </Sequence>
     <Workspace>
       <Local name="boolean"
         type='{"type":"bool"}'
         value="false"/>
     </Workspace>
   @endcode
 */
class PvAccessReadInstruction : public Instruction
{
public:
  PvAccessReadInstruction();
  ~PvAccessReadInstruction();

  static const std::string Type;

private:
  void SetupImpl(const Procedure& proc) override;

  ExecutionStatus ExecuteSingleImpl(UserInterface* ui, Workspace* ws) override;
};

}  // namespace sequencer

}  // namespace sup

#endif  // SUP_SEQUENCER_PLUGIN_EPICS_PV_ACCESS_READ_INSTRUCTION_H_
