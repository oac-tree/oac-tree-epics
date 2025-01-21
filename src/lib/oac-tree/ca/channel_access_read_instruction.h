/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP - oac-tree
 *
 * Description   : oac-tree for operational procedures
 *
 * Author        : Walter Van Herck (IO)
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

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_CHANNEL_ACCESS_READ_INSTRUCTION_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_CHANNEL_ACCESS_READ_INSTRUCTION_H_

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
 * @brief Instruction interfacing to an EPICS Channel Access Process Variable (PV).
 * @details The class provides a blocking read to EPICS CA. The instruction fails
 * in case the configured 'channel' can not be accessed within a timeout, which has a default
 * value of 2 seconds. Upon successful read, the specified workspace variable is updated.
 * The type of the workspace variable defines how the client-side tries and read the remote channel.
 *
 * @code
     <Sequence>
       <ChannelAccessRead name="get-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         timeout="5.0"
         varName="boolean"/>
       <ChannelAccessRead name="get-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         varName="uint32"/>
       <ChannelAccessRead name="get-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         varName="string"/>
     </Sequence>
     <Workspace>
       <Local name="boolean"
         type='{"type":"bool"}'
         value="false"/>
       <Local name="uint32"
         type='{"type":"uint32"}'
         value="0"/>
       <Local name="string"
         type='{"type":"string"}'
         value='"undefined"'/>
     </Workspace>
   @endcode
 *
 * @note EPICS CA support is provided through this class and also as asynchronous variables.
 */
class ChannelAccessReadInstruction : public Instruction
{
public:
  ChannelAccessReadInstruction();
  ~ChannelAccessReadInstruction();

  static const std::string Type;

private:
  ExecutionStatus ExecuteSingleImpl(UserInterface& ui, Workspace& ws) override;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_CHANNEL_ACCESS_READ_INSTRUCTION_H_
