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
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_WRITE_INSTRUCTION_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_WRITE_INSTRUCTION_H_

#include <sup/oac-tree/instruction.h>

#include <memory>

namespace sup
{
namespace dto
{
class AnyValue;
}  // namespace dto

namespace oac_tree
{
/**
 * @brief PvAccessWriteInstruction class.
 * @details Blocking instruction which establishes a PvAccess connection and updates the
 * channel with the value of a workspace variable. Mandatory attribute is 'channel' (PV name).
 * The value to be written can either be specified by referring to a variable in the
 * workspace ('varName' attribute) or by explicitly giving the type and value ('type' and
 * 'value' attributes). The optional 'timeout' attribute causes the instruction
 * to wait for connection with the specified timeout first and fails if connection was still not
 * established.
 * @code
     <Sequence>
       <PvAccessWrite name="write-pv"
         channel="EPICS::PVA::CHANNEL::BOOLEAN"
         varName="boolean"/>
     </Sequence>
     <Workspace>
       <Local name="boolean"
         type='{"type":"bool"}'
         value="false"/>
     </Workspace>
   @endcode
 */
class PvAccessWriteInstruction : public Instruction
{
public:
  PvAccessWriteInstruction();
  ~PvAccessWriteInstruction();

  static const std::string Type;

private:
  ExecutionStatus ExecuteSingleImpl(UserInterface& ui, Workspace& ws) override;

  sup::dto::AnyValue GetNewValue(UserInterface& ui, Workspace& ws) const;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_WRITE_INSTRUCTION_H_
