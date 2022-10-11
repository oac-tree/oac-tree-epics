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

#ifndef _SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_
#define _SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_

#include <string>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

bool SystemCall(const std::string& command);

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup

#endif // _SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_

