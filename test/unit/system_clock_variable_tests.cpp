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

#include <sequencer/misc/system_clock_variable.h>

#include <sup/sequencer/exceptions.h>
#include <sup/sequencer/variable.h>
#include <sup/sequencer/variable_registry.h>
#include <sup/sequencer/workspace.h>

#include <gtest/gtest.h>

using namespace sup::sequencer;

class SystemClockVariableTest : public ::testing::Test
{
protected:
  SystemClockVariableTest();
  ~SystemClockVariableTest();
};

TEST_F(SystemClockVariableTest, Setup)
{
  Workspace ws;
  {
    auto variable = GlobalVariableRegistry().Create("SystemClock");
    ASSERT_TRUE(static_cast<bool>(variable));
    EXPECT_NO_THROW(variable->Setup(ws));
    EXPECT_TRUE(variable->AddAttribute("format", "BAD_FORMAT"));
    EXPECT_THROW(variable->Setup(ws), VariableSetupException);
  }
  {
    auto variable = GlobalVariableRegistry().Create("SystemClock");
    ASSERT_TRUE(static_cast<bool>(variable));
    EXPECT_NO_THROW(variable->Setup(ws));
    EXPECT_TRUE(variable->AddAttribute("format", "uint64"));
    EXPECT_NO_THROW(variable->Setup(ws));
  }
  {
    auto variable = GlobalVariableRegistry().Create("SystemClock");
    ASSERT_TRUE(static_cast<bool>(variable));
    EXPECT_NO_THROW(variable->Setup(ws));
    EXPECT_TRUE(variable->AddAttribute("format", "ISO8601"));
    EXPECT_NO_THROW(variable->Setup(ws));
  }
}

TEST_F(SystemClockVariableTest, DefaultConstructed)
{
  Workspace ws;
  SystemClockVariable clock_var{};
  EXPECT_NO_THROW(clock_var.Setup(ws));
  EXPECT_EQ(clock_var.GetType(), "SystemClock");
  EXPECT_TRUE(clock_var.GetName().empty());

  sup::dto::AnyValue result{};
  EXPECT_TRUE(clock_var.GetValue(result));
  EXPECT_EQ(result.GetType(), sup::dto::UnsignedInteger64Type);
  EXPECT_NE(result.As<sup::dto::uint64>(), 0ul);
  EXPECT_FALSE(clock_var.SetValue(result));

  sup::dto::AnyValue string_result{sup::dto::StringType};
  EXPECT_FALSE(clock_var.GetValue(string_result));
  EXPECT_FALSE(clock_var.SetValue(string_result));
}

TEST_F(SystemClockVariableTest, TimestampFormat)
{
  Workspace ws;
  SystemClockVariable clock_var{};
  EXPECT_TRUE(clock_var.AddAttribute("format", "uint64"));
  EXPECT_NO_THROW(clock_var.Setup(ws));
  EXPECT_EQ(clock_var.GetType(), "SystemClock");
  EXPECT_TRUE(clock_var.GetName().empty());

  sup::dto::AnyValue result{};
  EXPECT_TRUE(clock_var.GetValue(result));
  EXPECT_EQ(result.GetType(), sup::dto::UnsignedInteger64Type);
  EXPECT_NE(result.As<sup::dto::uint64>(), 0ul);
  EXPECT_FALSE(clock_var.SetValue(result));

  sup::dto::AnyValue string_result{sup::dto::StringType};
  EXPECT_FALSE(clock_var.GetValue(string_result));
  EXPECT_FALSE(clock_var.SetValue(string_result));
}

TEST_F(SystemClockVariableTest, ISO8601Format)
{
  Workspace ws;
  SystemClockVariable clock_var{};
  EXPECT_TRUE(clock_var.AddAttribute("format", "ISO8601"));
  EXPECT_NO_THROW(clock_var.Setup(ws));
  EXPECT_EQ(clock_var.GetType(), "SystemClock");
  EXPECT_TRUE(clock_var.GetName().empty());

  sup::dto::AnyValue result{};
  EXPECT_TRUE(clock_var.GetValue(result));
  EXPECT_EQ(result.GetType(), sup::dto::StringType);
  auto time_str = result.As<std::string>();
  EXPECT_EQ(time_str.size(), 30u);
  EXPECT_EQ(time_str[4], '-');
  EXPECT_EQ(time_str[7], '-');
  EXPECT_EQ(time_str[10], 'T');
  EXPECT_EQ(time_str[13], ':');
  EXPECT_EQ(time_str[16], ':');
  EXPECT_EQ(time_str[19], '.');
  EXPECT_EQ(time_str[29], 'Z');
  EXPECT_FALSE(clock_var.SetValue(result));

  sup::dto::AnyValue uint64_result{sup::dto::UnsignedInteger64Type};
  EXPECT_FALSE(clock_var.GetValue(uint64_result));
  EXPECT_FALSE(clock_var.SetValue(uint64_result));
}

TEST_F(SystemClockVariableTest, UnknownFormat)
{
  Workspace ws;
  SystemClockVariable clock_var{};
  EXPECT_TRUE(clock_var.AddAttribute("format", "unknown"));
  EXPECT_THROW(clock_var.Setup(ws), VariableSetupException);
  EXPECT_EQ(clock_var.GetType(), "SystemClock");
  EXPECT_TRUE(clock_var.GetName().empty());

  sup::dto::AnyValue result{};
  EXPECT_FALSE(clock_var.GetValue(result));
  EXPECT_TRUE(sup::dto::IsEmptyValue(result));
  EXPECT_FALSE(clock_var.SetValue(result));
}

SystemClockVariableTest::SystemClockVariableTest() = default;

SystemClockVariableTest::~SystemClockVariableTest() = default;
