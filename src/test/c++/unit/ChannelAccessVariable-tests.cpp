/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP Sequencer
 *
 * Description   : Unit test code
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

#include <gtest/gtest.h> // Google test framework

#include <common/BasicTypes.h>
#include <common/TimeTools.h>
#include <common/AnyValueHelper.h>

#include <common/PVAccessServer.h>
#include <common/PVAccessTypes.h>

#include <SequenceParser.h>
#include <Variable.h>
#include <VariableRegistry.h>

// Local header files

#include "NullUserInterface.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "unit-test"

// Type declaration

// Function declaration

// Global variables

static ccs::log::Func_t _log_handler = ccs::log::SetStdout();

// Function definition

static inline bool Initialise (void)
{

  bool status = true;
  return status;

}

static inline bool Terminate (void)
{

  bool status = true;
  return status;

}

// Function definition

TEST(ChannelAccessVariable, GetValue_error)
{

  auto variable = sup::sequencer::GlobalVariableRegistry().Create("ChannelAccessVariable");

  bool status = static_cast<bool>(variable);

  if (status)
    { // Missing mandatory attribute .. Setup implicit with AddAttribute
      status = variable->AddAttribute("irrelevant","undefined");
    }

  ccs::types::AnyValue value; // Placeholder

  if (status)
    {
      status = ((false == variable->GetValue(value)) &&
                (false == static_cast<bool>(value.GetType())));
    }

  ASSERT_EQ(true, status);

}

#undef LOG_ALTERN_SRC
