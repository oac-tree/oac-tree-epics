/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP Sequencer
 *
 * Description   : Unit test code
 *
 * Author        : Gennady Pospelov (IO)
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

#include "PVClientVariable.h"

#include <VariableRegistry.h>
#include <gtest/gtest.h>

#include <algorithm>

using sup::sequencer::PVClientVariable;

class PVClientVariableTest : public ::testing::Test
{
};

TEST_F(PVClientVariableTest, VariableRegistration)
{
  auto registry = sup::sequencer::GlobalVariableRegistry();
  auto names = registry.RegisteredVariableNames();
  ASSERT_TRUE(std::find(names.begin(), names.end(), PVClientVariable::Type) != names.end());
  ASSERT_TRUE(dynamic_cast<PVClientVariable*>(registry.Create(PVClientVariable::Type).get()));
}

TEST_F(PVClientVariableTest, InvalidSetup)
{
  PVClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PVClientVariableTest:INTEGER"));
  EXPECT_THROW(variable.AddAttribute("datatype", "invalid-type"), std::runtime_error);
}

TEST_F(PVClientVariableTest, ValidSetup)
{
  PVClientVariable variable;
  EXPECT_NO_THROW(variable.AddAttribute("channel", "PVClientVariableTest:INTEGER"));
  EXPECT_NO_THROW(variable.AddAttribute("datatype", R"RAW({"type":"uint64"})RAW"));
}
