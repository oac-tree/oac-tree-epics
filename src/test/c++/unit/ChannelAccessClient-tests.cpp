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

#include <common/ChannelAccessClient.h>

// Local header files

#include "SystemCall.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "unit-test"

// Type declaration

// Function declaration

// Global variables

static ::ccs::log::Func_t _log_handler = ::ccs::log::SetStdout();

// Function definition

static inline bool Initialise (void)
{

  bool status = ::ccs::HelperTools::Exist("../resources/ChannelAccessClient.db");

  if (status)
    {
      status = ::ccs::HelperTools::ExecuteSystemCall(
        "/usr/bin/screen -d -m -S caclienttestIOC /usr/bin/softIoc -d ../resources/ChannelAccessClient.db &> /dev/null");
    }
  else
    {
      status = ::ccs::HelperTools::ExecuteSystemCall(
        "/usr/bin/screen -d -m -S caclienttestIOC /usr/bin/softIoc -d ./target/test/resources/ChannelAccessClient.db &> /dev/null");
    }

  if (status)
    {
      (void)::ccs::HelperTools::SleepFor(1000000000ul);
      status = ::ccs::HelperTools::ExecuteSystemCall("/usr/bin/caget SEQ-TEST:BOOL SEQ-TEST:FLOAT > /tmp/softioc-initialise.log");
    }

  if (status)
    {
      status = (NULL_PTR_CAST(::ccs::base::ChannelAccessClient*) != ::ccs::base::ChannelAccessInterface::GetInstance<::ccs::base::ChannelAccessClient>());
    }

  return status;

}

static inline bool Terminate (void)
{

  ::ccs::base::ChannelAccessInterface::Terminate<::ccs::base::ChannelAccessClient>();

  bool status = ::ccs::HelperTools::ExecuteSystemCall("/usr/bin/screen -S caclienttestIOC -X quit &> /dev/null");

  return status;

}

// Function definition

TEST(ChannelAccessClient, Launch)
{

  bool status = Initialise();

  ccs::base::SharedReference<ccs::types::AnyType> _type; // Placeholder

  if (status)
    {
      status = (0u < ccs::HelperTools::Parse(_type, "{\"type\":\"string\"}"));
    }

  if (status)
    {
      status = ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->AddVariable("SEQ-TEST:BOOL", ccs::types::AnyputVariable, _type);
    }

  if (status)
    {
      (void)ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->Launch();
      (void)ccs::HelperTools::SleepFor(1000000000ul);
    }

  if (status)
    { // Verify CA notification callbacks do  work
      status = (false == ccs::HelperTools::IsUndefinedString(reinterpret_cast<char*>(ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->GetVariable("SEQ-TEST:BOOL")->GetInstance())));
    }

  (void)Terminate();

  ASSERT_EQ(true, status);

}

#undef LOG_ALTERN_SRC
