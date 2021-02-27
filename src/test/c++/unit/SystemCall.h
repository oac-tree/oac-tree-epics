/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Supervision and Automation (SUP) Sequencer component
*
* Description   : Helper routines
*
* Author        : B.Bauvir (IO)
*
* Copyright (c) : 2010-2019 ITER Organization,
*		  CS 90 046
*		  13067 St. Paul-lez-Durance Cedex
*		  France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

#ifndef _SystemCall_h_
#define _SystemCall_h_

// Global header files

#include <cstdlib> // std::system, etc.

#include <common/BasicTypes.h>

#include <common/log-api.h>

// Local header files

// Constants

// Type definition

namespace ccs {

namespace HelperTools {

// Function declaration

static inline bool ExecuteSystemCall (const ccs::types::char8 * const command);

// Global variables

// Function definition

static inline bool ExecuteSystemCall (const ccs::types::char8 * const command)
{

  bool status = (0 == std::system(command));

  if (status)
    {
      log_info("ExecuteSystemCall - Invoking command '%s' successful", command);
    }
  else
    {
      log_error("ExecuteSystemCall - Error with '%s' command", command);
    }

  return status;

}

} // namespace HelperTools

} // namespace ccs

#endif // _SystemCall_h_

