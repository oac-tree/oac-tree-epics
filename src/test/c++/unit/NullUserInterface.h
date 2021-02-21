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

#ifndef _NullUserInterface_h_
#define _NullUserInterface_h_

// Global header files

#include <UserInterface.h>

// Local header files

// Constants

// Type definition

namespace sup {

namespace sequencer {

namespace gtest {

class NullUserInterface : public UserInterface
{

  private:

  protected:

  public:

    NullUserInterface (void);
    ~NullUserInterface (void) override;

    virtual void UpdateInstructionStatus (const Instruction * instruction);

};

// Global variables

// Function declaration

// Function definition

} // namespace gtest

} // namespace sequencer

} // namespace sup

#endif // _NullUserInterface_h_

