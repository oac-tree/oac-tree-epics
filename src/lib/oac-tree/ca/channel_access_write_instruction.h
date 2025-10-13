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
 * SPDX-License-Identifier: MIT
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file LICENSE located in the top level directory
 * of the distribution package.
******************************************************************************/

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_CHANNEL_ACCESS_WRITE_INSTRUCTION_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_CHANNEL_ACCESS_WRITE_INSTRUCTION_H_

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
class ChannelAccessPV;
}  // namespace epics


namespace oac_tree
{
/**
 * @brief Instruction interfacing to an EPICS Channel Access Process Variable (PV).
 * @details The class provides a blocking write to EPICS CA. The instruction fails
 * in case the configured 'channel' can not be accessed withing a timeout, which has a
 * default value of 2 seconds. The instruction provides two ways EPICS CA channels are updated:
 *
 *   Using 'type' and 'value' specification through attributes, or
 *   By reference to a workspace variable ('varName' attribute) holding the value to be written.
 *
 * @code
     <Sequence>
       <ChannelAccessWriteInstruction name="put-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         timeout="1.0"
         varName="boolean"/>
       <ChannelAccessFetchInstruction name="put-as-integer"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         type='{"type":"uint32"}'
         value="0"/>
       <ChannelAccessFetchInstruction name="put-as-string"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         type='{"type":"string"}'
         value='"FALSE"'/> <!-- As appropriate as per record specification --> <!-- Note the quotes -->
     </Sequence>
     <Workspace>
       <Local name="boolean"
         type='{"type":"bool"}'
         value="false"/>
     </Workspace>
   @endcode
 *
 * @note EPICS CA support is provided through this class and also as asynchronous variables.
 */
class ChannelAccessWriteInstruction : public Instruction
{
public:
  ChannelAccessWriteInstruction();
  ~ChannelAccessWriteInstruction() override;

  static const std::string Type;

private:
  std::string m_channel_name;
  sup::dto::AnyValue m_value;
  sup::dto::uint64 m_finish;
  std::unique_ptr<sup::epics::ChannelAccessPV> m_pv;

  bool InitHook(UserInterface& ui, Workspace& ws) override;

  ExecutionStatus ExecuteSingleImpl(UserInterface& ui, Workspace& ws) override;

  void ResetHook(UserInterface& ui) override;

  void HaltImpl() override;

  sup::dto::AnyValue GetNewValue(UserInterface& ui, Workspace& ws) const;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_CHANNEL_ACCESS_WRITE_INSTRUCTION_H_
