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

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <thread>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

bool SystemCall(const std::string& command)
{
  auto exit_status = std::system(command.c_str());
  return exit_status == 0 ? true : false;
}

bool BusyWaitFor(double timeout_sec, std::function<bool()> predicate)
{
  long timeout_ns = std::lround(timeout_sec * 1e9);
  auto time_end = std::chrono::system_clock::now() + std::chrono::nanoseconds(timeout_ns);
  while(!predicate() && std::chrono::system_clock::now() < time_end)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  return predicate();
}

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup
