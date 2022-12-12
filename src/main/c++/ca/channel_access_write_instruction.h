/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP - Sequencer
 *
 * Description   : Sequencer for operational procedures
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

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_CHANNEL_ACCESS_WRITE_INSTRUCTION_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_CHANNEL_ACCESS_WRITE_INSTRUCTION_H_

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
  ~ChannelAccessWriteInstruction();

  static const std::string Type;

private:
  double m_timeout_sec;

  void SetupImpl(const Procedure& proc) override;

  void ResetHook() override;

  ExecutionStatus ExecuteSingleImpl(UserInterface* ui, Workspace* ws) override;

  sup::dto::AnyValue GetNewValue(UserInterface* ui, Workspace* ws) const;
};

}  // namespace sequencer

}  // namespace sup

#endif  // SUP_SEQUENCER_PLUGIN_EPICS_CHANNEL_ACCESS_WRITE_INSTRUCTION_H_
