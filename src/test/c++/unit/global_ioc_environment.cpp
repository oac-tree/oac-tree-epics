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

#include "unit_test_helper.h"

#include <gtest/gtest.h>

class IOCEnvironment : public ::testing::Environment
{
public:
  ~IOCEnvironment();

  void SetUp() override;
  void TearDown() override;
};

::testing::Environment* const ioc_environment =
  ::testing::AddGlobalTestEnvironment(new IOCEnvironment);

IOCEnvironment::~IOCEnvironment() = default;

void IOCEnvironment::SetUp()
{
  sup::sequencer::unit_test_helper::StartIOC("ChannelAccessClient.db", "seq_plugin_testIOC");
}

void IOCEnvironment::TearDown()
{
  sup::sequencer::unit_test_helper::StopIOC("seq_plugin_testIOC");
}
