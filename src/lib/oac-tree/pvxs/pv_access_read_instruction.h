/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Walter Van Herck
 *
 * Copyright (c) : 2010-2025 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 * SPDX-License-Identifier: MIT
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file LICENSE located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_READ_INSTRUCTION_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_READ_INSTRUCTION_H_

#include <sup/oac-tree/instruction.h>

#include <memory>

namespace sup
{
namespace dto
{
class AnyValue;
}  // namespace dto
namespace epics
{
class PvAccessClientPV;
}  // namespace epics

namespace oac_tree
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
         outputVar="boolean"/>
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
  ~PvAccessReadInstruction() override;

  static const std::string Type;

private:
  std::string m_channel_name;
  sup::dto::uint64 m_finish;
  std::unique_ptr<sup::epics::PvAccessClientPV> m_pv;

  bool InitHook(UserInterface& ui, Workspace& ws) override;

  ExecutionStatus ExecuteSingleImpl(UserInterface& ui, Workspace& ws) override;

  void ResetHook(UserInterface& ui) override;

  void HaltImpl() override;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_READ_INSTRUCTION_H_
