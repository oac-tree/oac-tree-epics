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

#if !defined CPP_COMMON_EPICS_VERSION || CPP_COMMON_EPICS_VERSION <= 133

struct ca_client_context; // Forward type definition

// Function declaration

namespace ccs {

namespace HelperTools {

namespace ChannelAccess {
#if 0
static inline bool CreateContext (bool preempt) // CA wrappers
{

  log_trace("%s - Entering method", __FUNCTION__);
  
  // Create CA context
  log_debug("%s - Create CA context with '%s' preemptive callbacks", __FUNCTION__, (preempt ? "enabled" : "disabled"));
  bool status = (ECA_NORMAL == ca_context_create((preempt ? ca_enable_preemptive_callback : ca_disable_preemptive_callback)));

  log_trace("%s - Leaving method", __FUNCTION__);

  return status;

}
#endif
static inline bool AttachContext (struct ca_client_context* context) // CA wrappers
{

  log_trace("%s - Entering method", __FUNCTION__);
  
  // Create CA context
  log_debug("%s - Attach to CA context", __FUNCTION__);
  int _status = ca_attach_context(context);
  bool status = ((ECA_NORMAL == _status) || (ECA_ISATTACHED == _status));

  log_trace("%s - Leaving method", __FUNCTION__);

  return status;

}

static inline struct ca_client_context* GetContext (void) // CA wrappers
{

  log_trace("%s - Entering method", __FUNCTION__);
  
  // Retrieve CA context
  struct ca_client_context* ret = ca_current_context();

  log_trace("%s - Leaving method", __FUNCTION__);

  return ret;

}

static inline bool DetachContext (void) // CA wrappers
{

  log_trace("%s - Entering method", __FUNCTION__);
  
  // Create CA context
  log_debug("%s - Detach from CA context", __FUNCTION__);
  ca_detach_context();

  log_trace("%s - Leaving method", __FUNCTION__);

  return true;

}

} // namespace ChannelAccess

} // namespace HelperTools

} // namespace ccs

#endif

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
