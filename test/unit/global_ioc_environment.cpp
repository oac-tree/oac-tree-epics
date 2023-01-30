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
#include "softioc_runner.h"
#include "softioc_utils.h"

#include <gtest/gtest.h>

class IOCEnvironment : public ::testing::Environment
{
public:
  IOCEnvironment();
  ~IOCEnvironment();

  void SetUp() override;
  void TearDown() override;

  sup::sequencer::softioc_utils::SoftIocRunner m_softioc_runner;
};

::testing::Environment* const ioc_environment =
  ::testing::AddGlobalTestEnvironment(new IOCEnvironment);

IOCEnvironment::IOCEnvironment()
  : m_softioc_runner{"seq-plugin-channel-access-tests"}
{}

IOCEnvironment::~IOCEnvironment() = default;

void IOCEnvironment::SetUp()
{
  m_softioc_runner.Start(sup::sequencer::softioc_utils::GetEpicsDBContentString());
}

void IOCEnvironment::TearDown()
{
  m_softioc_runner.Stop();
}
