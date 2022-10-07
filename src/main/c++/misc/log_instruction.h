/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP - DTO
 *
 * Description   : Data transfer objects for SUP
 *
 * Author        : Walter Van Herck (IO)
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

#ifndef _SUP_SEQUENCER_PLUGIN_EPICS_LOG_INSTRUCTION_H_
#define _SUP_SEQUENCER_PLUGIN_EPICS_LOG_INSTRUCTION_H_

#include <sup/sequencer/instruction.h>

namespace sup
{
namespace sequencer
{

/**
 * @brief Outputs a message to the sequencer log.
 */
class LogInstruction : public Instruction
{
public:
  LogInstruction();
  ~LogInstruction() override;

  static const std::string Type;

private:
  bool SetupImpl(const Procedure& proc);
  ExecutionStatus ExecuteSingleImpl(UserInterface * ui, Workspace * ws) override;
};

}  // namespace sequencer

}  // namespace sup

#endif  // _SUP_SEQUENCER_PLUGIN_EPICS_LOG_INSTRUCTION_H_
