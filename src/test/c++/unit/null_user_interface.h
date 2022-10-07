/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : UserInterface implementation
 *
 * Author        : B.Bauvir (IO)
 *
 * Copyright (c) : 2010-2019 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
******************************************************************************/

#ifndef _SUP_SEQUENCER_PLUGIN_EPICS_NULL_INTERFACE_H_
#define _SUP_SEQUENCER_PLUGIN_EPICS_NULL_INTERFACE_H_

#include <sup/sequencer/user_interface.h>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

class NullUserInterface : public UserInterface
{
public:
  NullUserInterface();
  ~NullUserInterface() override;

  virtual void UpdateInstructionStatusImpl(const Instruction* instruction);
};

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup

#endif // _SUP_SEQUENCER_PLUGIN_EPICS_NULL_INTERFACE_H_

