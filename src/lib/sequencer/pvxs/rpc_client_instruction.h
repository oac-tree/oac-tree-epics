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

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_RPC_CLIENT_INSTRUCTION_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_RPC_CLIENT_INSTRUCTION_H_

#include <sup/sequencer/instruction.h>

namespace sup
{
namespace dto
{
class AnyValue;
}

namespace sequencer
{
/**
 * @brief RPC client instruction.
 * @details The instruction provides Remote Procedure Call (RPC) support to a named 'service',
 * sending a request from a named workspace 'request' variable or using 'type' and 'value'
 * attributes. The RPC call is made with a timeout that is given by the 'timeout' attribute or
 * the default value for the underlying RPC implementation.
 * The reply is written back to the workspace if specified.
 *
 * @code
     <RPCClient name="rpc-client"
                service="rpc@plant-system"
                request="request" reply="reply"
                timeout="4.0"/>
     <Workspace>
       <Local name="request"
         type='{"type":"Request_t","attributes":[{"qualifier":{"type":"string"}},{"value":{"type":"uint32"}},{"enable":{"type":"bool"}}]}'
         value='{"qualifier":"configure","value":1234,"enable":true}'/>
       <Local name="reply"/>
     </Workspace>
   @endcode
 */
class RPCClientInstruction : public Instruction
{
public:
  RPCClientInstruction();
  ~RPCClientInstruction();

  static const std::string Type;

private:
  ExecutionStatus ExecuteSingleImpl(UserInterface& ui, Workspace& ws) override;

  sup::dto::AnyValue GetRequest(UserInterface& ui, Workspace& ws);
};

}  // namespace sequencer

}  // namespace sup

#endif  // SUP_SEQUENCER_PLUGIN_EPICS_RPC_CLIENT_INSTRUCTION_H_
