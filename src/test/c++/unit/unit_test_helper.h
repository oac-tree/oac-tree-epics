/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : UserInterface implementation
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

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_

#include <functional>
#include <string>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

bool BusyWaitFor(double timeout_sec, std::function<bool()> predicate);

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup

#endif // SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_

