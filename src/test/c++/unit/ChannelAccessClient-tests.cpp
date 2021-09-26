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

static const auto ONE_SECOND = 1000000000ul;

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
  (void)ccs::HelperTools::SleepFor(ONE_SECOND);

  ccs::base::SharedReference<ccs::types::AnyType> _type;

  ASSERT_TRUE(0u < ccs::HelperTools::Parse(_type, "{\"type\":\"string\"}"));
  ASSERT_TRUE(ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()
    ->AddVariable("SEQ-TEST:BOOL", ccs::types::AnyputVariable, _type));
  (void)ccs::base::ChannelAccessInterface::GetInstance<ccs::base::ChannelAccessClient>()->Launch();
  (void)ccs::HelperTools::SleepFor(ONE_SECOND);
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

TEST_F(ChannelAccessClientTest, ReadWriteClients)
{
  ccs::HelperTools::SleepFor(ONE_SECOND);

  // construct clients
  ccs::base::ChannelAccessClient ca_client;
  ccs::base::ChannelAccessClient ca_client_reader;

  // preparing variable
  ASSERT_TRUE(ca_client.AddVariable("SEQ-TEST:FLOAT", ccs::types::AnyputVariable, ccs::types::Float32));
  ASSERT_TRUE(ca_client_reader.AddVariable("SEQ-TEST:FLOAT", ccs::types::AnyputVariable, ccs::types::Float32));

  // starting clients
  ASSERT_TRUE(ca_client.Launch());
  ASSERT_TRUE(ca_client_reader.Launch());

  // set first value
  const ccs::types::float32 value1 = 3.5f;
  ASSERT_TRUE(ca_client.SetVariable("SEQ-TEST:FLOAT", value1));

  ccs::HelperTools::SleepFor(ONE_SECOND);

  // reading variable through first client
  ccs::types::float32 variable_float = 0.0f;
  EXPECT_TRUE(ca_client.GetVariable("SEQ-TEST:FLOAT", variable_float));
  EXPECT_FLOAT_EQ(variable_float, value1);

  // reading variable through second client
  variable_float = 0.0f;
  EXPECT_TRUE(ca_client_reader.GetVariable("SEQ-TEST:FLOAT", variable_float));
  EXPECT_FLOAT_EQ(variable_float, value1);

  // set second value
  const ccs::types::float32 value2 = -1.5f;
  ASSERT_TRUE(ca_client.SetVariable("SEQ-TEST:FLOAT", value2));

  ccs::HelperTools::SleepFor(ONE_SECOND);

  // reading variable through first client
  variable_float = 0.0f;
  EXPECT_TRUE(ca_client.GetVariable("SEQ-TEST:FLOAT", variable_float));
  EXPECT_FLOAT_EQ(variable_float, value2);

  // reading variable through second client
  variable_float = 0.0f;
  EXPECT_TRUE(ca_client_reader.GetVariable("SEQ-TEST:FLOAT", variable_float));
  EXPECT_FLOAT_EQ(variable_float, value2);

  // reset clients
  ASSERT_TRUE(ca_client.Reset());
  ASSERT_TRUE(ca_client_reader.Reset());
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
