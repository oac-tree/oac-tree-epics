/******************************************************************************
* $HeadURL: $
* $Id: $
*
* Project       : CODAC Core System
*
* Description   : Helper routines
*
* Author        : Bertrand Bauvir (IO)
*
* Copyright (c) : 2010-2021 ITER Organization,
*		  CS 90 046
*		  13067 St. Paul-lez-Durance Cedex
*		  France
*
* This file is part of ITER CODAC software.
* For the terms and conditions of redistribution or use of this software
* refer to the file ITER-LICENSE.TXT located in the top level directory
* of the distribution package.
******************************************************************************/

#ifndef _ToInteger_h_
#define _ToInteger_h_

// Global header files

#include <common/BasicTypes.h> // Misc. type definition

#include <common/StringTools.h> // Misc. helper routines

// Local header files

// Constants

// Type definition

namespace ccs {

namespace HelperTools {

// Function declaration

template <typename Type> inline Type ToInteger (const ccs::types::char8 * const buffer);

// Global variables

// Function definition

template <> inline ccs::types::uint64 ToInteger<ccs::types::uint64> (const ccs::types::char8 * const buffer)
{

  ccs::types::uint64 ret = 0ul;

  bool status = (ccs::HelperTools::IsIntegerString(buffer) == true);

  if (status)
    {
      ccs::types::char8* p = NULL_PTR_CAST(ccs::types::char8*);
      ret = static_cast<ccs::types::uint64>(strtoul(buffer, &p, 10));
    }

  return ret;

}

} // namespace HelperTools

} // namespace ccs

#endif // _ToInteger_h_

