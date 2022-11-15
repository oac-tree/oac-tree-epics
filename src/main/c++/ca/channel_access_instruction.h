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

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_CHANNEL_ACCESS_INSTRUCTION_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_CHANNEL_ACCESS_INSTRUCTION_H_

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
 * @details The class provides a blocking read to EPICS CA. The instruction fails
 * in case the configured 'channel' can not be accessed. Upon successful read, the
 * specified workspace variable is updated. The type of the workspace variable
 * defines how the client-side tries and read the remote channel.
 *
 * @code
     <Sequence>
       <ChannelAccessRead name="get-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
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
  double m_timeout_sec;
  /**
   * @brief See sup::sequencer::Instruction.
   * @details Verify and handle attributes.
   */
  bool SetupImpl(const Procedure& proc) override;

  /**
   * @brief See sup::sequencer::Instruction.
   */
  void ResetHook() override;

  /**
   * @brief See sup::sequencer::Instruction.
   * @details Connects to the specified 'channel' and reads the value into the
   * workspace variable with name 'varName'.
   */
  ExecutionStatus ExecuteSingleImpl(UserInterface* ui, Workspace* ws) override;
};

/**
 * @brief Instruction interfacing to an EPICS Channel Access Process Variable (PV).
 * @details The class provides a blocking write to EPICS CA. The instruction fails
 * in case the configured 'channel' can not be accessed. The instruction provides
 * two ways EPICS CA channels are updated:
 *
 *   Using 'type' and 'value' specification through attributes, or
 *   By reference to a workspace variable ('varName' attribute) holding the value to be written.
 *
 * The EPICS CA connection is verified after an optional 'timeout' period specified in
 * ns resolution which is defaulted to 100ms.
 * @code
     <Sequence>
       <ChannelAccessWriteInstruction name="put-client"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         timeout="100000000"
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
 * Procedures mixing asynchronous handling and synchronous instructions have not been tested.
 * @note A single EPICS CA context is created for the sequencer procedure and shared among
 * all instruction instances. An explicit context attach/detach is performed by each call to
 * Instruction::ExecuteSingleImpl in order to allow for multi-threaded operation.
 */
class ChannelAccessWriteInstruction : public Instruction
{
public:
  ChannelAccessWriteInstruction();
  ~ChannelAccessWriteInstruction();

  static const std::string Type;

private:
  double m_timeout_sec;

  /**
   * @brief See sup::sequencer::Instruction.
   */
  bool SetupImpl(const Procedure& proc) override;

  /**
   * @brief See sup::sequencer::Instruction.
   */
  void ResetHook() override;

  /**
   * @brief See sup::sequencer::Instruction.
   */
  ExecutionStatus ExecuteSingleImpl(UserInterface* ui, Workspace* ws) override;

  sup::dto::AnyValue GetNewValue(Workspace* ws) const;
};

}  // namespace sequencer

}  // namespace sup

#endif  // SUP_SEQUENCER_PLUGIN_EPICS_CHANNEL_ACCESS_INSTRUCTION_H_
