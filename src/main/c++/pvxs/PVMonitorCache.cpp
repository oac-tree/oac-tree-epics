/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : Instruction node implementation
 *
 * Author        : Bertrand Bauvir (IO)
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

// Global header files

#include <common/BasicTypes.h>  // Misc. type definition
#include <common/PVMonitor.h>   // Plug-in managed through dynamic linking .. see Makefile

#include <mutex>  // std::mutex, etc.

#include <common/log-api.h>  // Syslog wrapper routines

// Local header files

#include "PVMonitorCache.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup
{
namespace sequencer
{
// Function declaration

// Global variables

// Function definition

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
void PVMonitorCache::HandleEvent(const ccs::base::PVMonitor::Event& event)
{
  if (ccs::base::PVMonitor::Event::Connect == event)
  {
    log_notice("PVMonitorCache::HandleEvent - Connect to '%s'", ccs::base::PVMonitor::GetChannel());
  }

  if (_initialised && (ccs::base::PVMonitor::Event::Disconnect == event))
  {
    log_notice("PVMonitorCache::HandleEvent - Disconnect from '%s'",
               ccs::base::PVMonitor::GetChannel());
    _initialised = false;
  }

  return;
}

// cppcheck-suppress unusedFunction // Callbacks used in a separate translation unit
void PVMonitorCache::HandleMonitor(const ccs::types::AnyValue& value)
{
  bool status = static_cast<bool>(value.GetType());

  if (status)
  {
    _initialised = true;
  }

  if (status)
  {  // MUTEX wrt. GetValue
    std::lock_guard<std::mutex> lock(_async_mutex);
    _value = value;
  }

  if (status)
  {
    if (_callback)
    {
      _callback(value);
    }
    ccs::types::char8 buffer[2048];
    (void)_value.SerialiseInstance(buffer, 2048u);
    log_info("PVMonitorCache::HandleMonitor - Storing '%s' value", buffer);
  }

  return;
}

bool PVMonitorCache::IsInitialised(void) const
{
  return _initialised;
}

bool PVMonitorCache::SetChannel(const ccs::types::char8* const name)
{
  bool status = (false == _initialised);

  if (status)
  {
    status = ccs::base::PVMonitor::SetChannel(name);
  }

  if (status)
  {  // Instantiate implementation
    status = ccs::base::PVMonitor::Initialise();
  }

  return status;
}

bool PVMonitorCache::SetCallback(const ccs::types::char8* name,
                                 const std::function<void(const ccs::types::AnyValue&)>& cb)
{
  _callback = cb;
  return true;
}

bool PVMonitorCache::GetValue(ccs::types::AnyValue& value) const
{
  bool status = _initialised;

  if (status)
  {  // MUTEX wrt. HandleMonitor
    std::lock_guard<std::mutex> lock(_async_mutex);
    value = _value;
  }

  return status;
}

PVMonitorCache::PVMonitorCache(void) : ccs::base::PVMonitor() {}
PVMonitorCache::~PVMonitorCache(void) {}

}  // namespace sequencer

}  // namespace sup

#undef LOG_ALTERN_SRC
