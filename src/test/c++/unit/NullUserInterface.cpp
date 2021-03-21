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

// Global header files

#include <UserInterface.h>

// Local header files

#include "NullUserInterface.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "sup::sequencer"

// Type definition

namespace sup {

namespace sequencer {

namespace gtest {

// Function declaration

// Global variables

// Function definition

void NullUserInterface::UpdateInstructionStatusImpl (const Instruction * instruction) {}
NullUserInterface::NullUserInterface (void) {}
NullUserInterface::~NullUserInterface (void) {}

} // namespace gtest

} // namespace sequencer

} // namespace sup

#undef LOG_ALTERN_SRC
