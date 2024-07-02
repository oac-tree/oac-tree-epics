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

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_CHANNEL_ACCESS_CLIENT_VARIABLE_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_CHANNEL_ACCESS_CLIENT_VARIABLE_H_

#include <sup/sequencer/variable.h>

#include <memory>

namespace sup
{
namespace epics
{
class ChannelAccessPV;
}  // namespace epics

namespace sequencer
{

/**
 * @brief Workspace variable interface to EPICS Channel Access Process Variable (PV).
 * @details The class interfaces to ccs::base::ChannelAccessClient to manage asynchronous
 * handling of EPICS CA connections, notification and update. The variable is configured
 * with mandatory 'channel' (PV name) and 'type' attributes. The 'type' attribute
 * constraints the client-side type to use for get/put requests; e.g. enumeration-type
 * IOC records such as mbbi/mbbo can be accessed as integer or string.
 * The implementation provides as well a way to change the CA client thread period from
 * default using an optional 'period' attribute with ns resolution.
 *
 * @code
     <Workspace>
       <ChannelAccessVariable name="boolean"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         type='{"type":"bool"}'
         period="10000000"/>
       <ChannelAccessVariable name="boolean-as-string"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         type='{"type":"string"}'/>
       <ChannelAccessVariable name="boolean-as-integer"
         channel="EPICS::CA::CHANNEL::BOOLEAN"
         type='{"type":"uint32"}'/>
       <ChannelAccessVariable name="integer"
         channel="EPICS::CA::CHANNEL::INTEGER"
         type='{"type":"uint32"}'/>
       <ChannelAccessVariable name="array"
         channel="EPICS::CA::CHANNEL::ARRAY"
         type='{"type":"uint32[]","multiplicity":8,"element":{"type":"uint32"}}'/>
     </Workspace>
   @endcode
 *
 * @note Multiple variable instances use the same singleton CA client instance which is
 * created and terminated as neceessary when variable instances come into procedure scope
 * and leave it.
 * @note The singleton CA client instance ignores new requests for already connected channels.
 * The example above would not currently run as such as the 'boolean-as-string' and 'boolean-as
 * -integer' would be ignored by the CA client implementation by reason of the channel name
 * to have already been registered. This implementation limitation may be lifted in the future
 * if required.
 * @note EPICS CA support is provided through this class and also as blocking instructions.
 * Procedures mixing asynchronous handling using this class and synchronous instructions have
 * not been tested.
 */
class ChannelAccessClientVariable : public Variable
{
public:
  ChannelAccessClientVariable();
  ~ChannelAccessClientVariable() override;

  static const std::string Type;

private:
  bool GetValueImpl(sup::dto::AnyValue& value) const override;
  bool SetValueImpl(const sup::dto::AnyValue& value) override;
  bool IsAvailableImpl() const override;
  void SetupImpl(const Workspace& ws) override;
  void TeardownImpl() override;
  std::unique_ptr<epics::ChannelAccessPV> m_pv;
  sup::dto::AnyType m_type;
};

}  // namespace sequencer

}  // namespace sup

#endif  // SUP_SEQUENCER_PLUGIN_EPICS_CHANNEL_ACCESS_CLIENT_VARIABLE_H_
