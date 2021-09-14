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

class ChannelAccessClientTest : public ::testing::Test
{
protected:
  ChannelAccessClientTest();
  virtual ~ChannelAccessClientTest();

  void StopIOC();

  bool init_success;
};

// Function definition

TEST_F(ChannelAccessClientTest, Launch)
{
  ASSERT_TRUE(init_success);
  (void)ccs::HelperTools::SleepFor(1000000000ul);

  ccs::base::SharedReference<ccs::types::AnyType> _type;

  ASSERT_TRUE(0u < ccs::HelperTools::Parse(_type, "{\"type\":\"string\"}"));
  ASSERT_TRUE(ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()
    ->AddVariable("SEQ-TEST:BOOL", ccs::types::AnyputVariable, _type));
  (void)ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->Launch();
  (void)ccs::HelperTools::SleepFor(1000000000ul);
  ccs::types::AnyValue value;
  EXPECT_TRUE(ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()
      ->GetAnyValue("SEQ-TEST:BOOL", value));
  ccs::base::ChannelAccessInterface::Terminate<ccs::base::ChannelAccessClient>();
  ccs::types::char8 buffer [1024];
  ccs::HelperTools::SerialiseToJSONStream(&value, buffer, 1024u);
  log_info("TEST_F(ChannelAccessClientTest, Launch) - Value is ..");
  log_info("'%s'", buffer);
  EXPECT_FALSE(ccs::HelperTools::IsUndefinedString(reinterpret_cast<char*>(value.GetInstance())));
}

ChannelAccessClientTest::ChannelAccessClientTest()
  : init_success{false}
{
  if (::ccs::HelperTools::Exist("../resources/ChannelAccessClient.db"))
  {
    init_success = ::ccs::HelperTools::ExecuteSystemCall(
        "/usr/bin/screen -d -m -S caclienttestIOC /usr/bin/softIoc -d ../resources/ChannelAccessClient.db &> /dev/null");
  }
  else
  {
    init_success = ::ccs::HelperTools::ExecuteSystemCall(
        "/usr/bin/screen -d -m -S caclienttestIOC /usr/bin/softIoc -d ./target/test/resources/ChannelAccessClient.db &> /dev/null");
  }
}

ChannelAccessClientTest::~ChannelAccessClientTest()
{
  StopIOC();
}

void ChannelAccessClientTest::StopIOC()
{
  if (init_success)
  {
    ::ccs::HelperTools::ExecuteSystemCall("/usr/bin/screen -S caclienttestIOC -X quit &> /dev/null");
    init_success = false;
  }
}

#undef LOG_ALTERN_SRC
