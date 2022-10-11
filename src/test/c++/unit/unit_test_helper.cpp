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
* Copyright (c) : 2010-2020 ITER Organization,
*                 CS 90 046
*                 13067 St. Paul-lez-Durance Cedex
*                 France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

#include "unit_test_helper.h"

#include <cstdlib>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

bool SystemCall(const std::string& command)
{
  auto exit_status = std::system(command.c_str());
  return exit_status == 0 ? true : false;
}

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup
