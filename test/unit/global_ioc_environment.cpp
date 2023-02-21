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

#include <sup/epics-test/softioc_runner.h>
#include <sup/epics-test/softioc_utils.h>

#include <gtest/gtest.h>

namespace
{
const std::string db_content = R"RAW(
record (bo,"SEQ-TEST:BOOL")
{
    field(DESC,"Some EPICSv3 record")
    field(ONAM,"TRUE")
    field(OSV,"NO_ALARM")
    field(ZNAM,"FALSE")
    field(ZSV,"NO_ALARM")
    field(VAL,"0")
    field(PINI, "YES")
}

record (ao,"SEQ-TEST:FLOAT")
{
    field(DESC,"Some EPICSv3 record")
    field(DRVH,"5.0")
    field(DRVL,"-5.0")
    field(VAL,"0")
    field(PINI, "YES")
}

record (stringout,"SEQ-TEST:STRING")
{
    field(DESC,"Some EPICSv3 record")
    field(PINI, "YES")
}

record (waveform,"SEQ-TEST:UIARRAY")
{
    field(DESC,"Some EPICSv3 record")
    field(FTVL, "ULONG")
    field(NELM, "8")
    field(PINI, "YES")
}

record (waveform,"SEQ-TEST:CHARRAY")
{
    field(DESC,"Some EPICSv3 record")
    field(FTVL, "CHAR")
    field(NELM, "1024")
    field(PINI, "YES")
}
)RAW";

std::string GetEpicsDBContentString()
{
  return db_content;
}

}  // unnamed namespace

class IOCEnvironment : public ::testing::Environment
{
public:
  IOCEnvironment();
  ~IOCEnvironment();

  void SetUp() override;
  void TearDown() override;

  sup::epics::test::SoftIocRunner m_softioc_runner;
};

::testing::Environment* const ioc_environment =
  ::testing::AddGlobalTestEnvironment(new IOCEnvironment);

IOCEnvironment::IOCEnvironment()
  : m_softioc_runner{"seq-plugin-channel-access-tests"}
{}

IOCEnvironment::~IOCEnvironment() = default;

void IOCEnvironment::SetUp()
{
  m_softioc_runner.Start(GetEpicsDBContentString());
}

void IOCEnvironment::TearDown()
{
  m_softioc_runner.Stop();
}
