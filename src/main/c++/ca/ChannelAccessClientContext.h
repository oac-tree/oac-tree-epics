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

#ifndef _ChannelAccessClientContext_h_
#define _ChannelAccessClientContext_h_

// Global header files

#include <common/BasicTypes.h> // Misc. type definition

#include <common/log-api.h> // Syslog wrapper routines

// Local header files

// Constants

// Type definition

namespace ccs {

namespace HelperTools {

namespace ChannelAccessClientContext {

// Function declaration

bool Attach (void);
bool Detach (void);

// Global variables

// Function definition

} // namespace ChannelAccessClientContext

} // namespace HelperTools

} // namespace ccs

#endif // _ChannelAccessClientContext_h_

