/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Core System
*
* Description   : EPICS ChannelAccess helper
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

// Global header files

#include <common/BasicTypes.h> // Misc. type definition

#include <common/log-api.h> // Syslog wrapper routines

#include <common/StringTools.h> // Missing from ChannelAccessHelper.h
#include <common/ChannelAccessHelper.h>

// Local header files

#include "ChannelAccessClientContext.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "ca-if"

// Type definition

namespace ccs {

namespace HelperTools {

namespace ChannelAccessClientContext {

static inline bool IsInitialised (void);

// Global variables

static ca_client_context* _ca_context = NULL_PTR_CAST(ca_client_context*);
static ccs::types::uint32 _ca_client_count = 0u;

// Function definition

bool CreateAsNecessary (void)
{

  log_debug("ChannelAccessClientContext::Create - Method called with '%u' counter", _ca_client_count);

  bool status = IsInitialised();

  if (!status)
    { // Reference counted client instances
      log_notice("ChannelAccessClientContext::Create - Initialise the shared reference ..");
      status = ::ccs::HelperTools::ChannelAccess::CreateContext(true);
      _ca_context = ::ccs::HelperTools::ChannelAccess::GetContext();
    }

  // Increment count
  _ca_client_count += 1u;

  return status;

}

static inline bool IsInitialised (void)
{

  return (NULL_PTR_CAST(ca_client_context*) != _ca_context);

}

bool Attach (void)
{

  bool status = CreateAsNecessary();

  if (status)
    {
      log_notice("ChannelAccessClientContext::Attach - Connect to the shared context..");
      status = ::ccs::HelperTools::ChannelAccess::AttachContext(_ca_context);
    }

  return status;

}

bool Detach (void)
{

  bool status = IsInitialised();

  if (status)
    { // Reference counted ccs::base::ChannelAccessClient instance
      log_notice("ChannelAccessClientContext::Detach - Detach from the shared context..");
      ::ccs::HelperTools::ChannelAccess::DetachContext();
      status = TerminateWhenAppropriate();
    }

  return status;

}

bool TerminateWhenAppropriate (void)
{

  log_debug("ChannelAccessClientContext::Terminate - Method called with '%u' counter", _ca_client_count);

  bool status = IsInitialised();

  if (0u < _ca_client_count)
    { // Decrement count
      _ca_client_count -= 1u;
    }

  if (status && (0u == _ca_client_count))
    { // Assume destroying the reference is sufficient for context tear-down
      log_notice("ChannelAccessClientContext::Terminate - Forgetting the shared reference ..");
      ::ccs::HelperTools::ChannelAccess::ClearContext();
      _ca_context = NULL_PTR_CAST(ca_client_context*);
    }

  return status;

}

} // namespace ChannelAccessClientContext

} // namespace HelperTools

} // namespace ccs

#undef LOG_ALTERN_SRC
