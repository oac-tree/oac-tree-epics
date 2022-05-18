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
static ccs::base::PVAccessServer* _pva_srv = static_cast<ccs::base::PVAccessServer*>(NULL);

// Function definition

static inline bool Initialise()
{
  _pva_srv = ccs::base::PVAccessInterface::GetInstance<ccs::base::PVAccessServer>();

  bool status = (static_cast<ccs::base::PVAccessServer*>(NULL) != _pva_srv);
  if (status)
  {  // Create variable
    status = _pva_srv->AddVariable("sequencer::test::pva-variable", ccs::types::AnyputVariable,
                                   ccs::base::PVAccessTypes::Collection_int);
  }
  if (status)
  {
    status = _pva_srv->SetPeriod(10000000ul);
  }
  if (status)
  {
    status = _pva_srv->Launch();
  }
  if (status)
  {  // Let the infrastructure settle
    (void)ccs::HelperTools::SleepFor(1000000000ul);
  }
  if (status)
  {
    status = _pva_srv->IsValid("sequencer::test::pva-variable");
  }
  return status;
}

static inline bool Terminate()
{
  bool status = (static_cast<ccs::base::PVAccessServer*>(NULL) != _pva_srv);
  if (status)
  {
    ccs::base::PVAccessInterface::Terminate<ccs::base::PVAccessServer>();
    _pva_srv = static_cast<ccs::base::PVAccessServer*>(NULL);
  }
  if (status)
  {  // Let the infrastructure settle
    (void)ccs::HelperTools::SleepFor(1000000000ul);
  }
  return status;
}

// Function definition

TEST(PVMonitorVariable, GetValue_error)
{
  auto variable = sup::sequencer::GlobalVariableRegistry().Create("PVMonitorVariable");

  bool status = static_cast<bool>(variable);
  if (status)
  {  // Missing mandatory attribute ..
    status = variable->AddAttribute("irrelevant", "undefined");
    variable->Setup();
  }

  ccs::types::AnyValue value;  // Placeholder
  if (status)
  {
    status =
        ((false == variable->GetValue(value)) && (false == static_cast<bool>(value.GetType())));
  }
  if (status)
  {
    status = variable->AddAttribute("channel", "sequencer::test::pva-variable");
    variable->Setup();
  }
  if (status)
  {  // No such variable available yet
    status =
        ((false == variable->GetValue(value)) && (false == static_cast<bool>(value.GetType())));
  }

  ASSERT_EQ(true, status);
}

TEST(PVMonitorVariable, GetValue_success)
{
  auto variable = sup::sequencer::GlobalVariableRegistry().Create("PVMonitorVariable");

  bool status = static_cast<bool>(variable);

  if (status)
  {
    status = variable->AddAttribute("channel", "sequencer::test::pva-variable");
    variable->Setup();
  }

  ccs::types::AnyValue value;  // Placeholder
  if (status)
  {  // Create PVA server and variable
    status = Initialise();
  }
  if (status)
  {
    status = variable->GetValue(value);
  }
  if (status)
  {  // Test variable
    status = static_cast<bool>(value.GetType());
  }
  if (status)
  {  // Update PVA variable
    ccs::types::uint32 value = 1234u;
    status =
        (ccs::HelperTools::SetAttributeValue(_pva_srv->GetVariable("sequencer::test::pva-variable"),
                                             "scalars.uint32", value)
         && ccs::HelperTools::SetAttributeValue(
             _pva_srv->GetVariable("sequencer::test::pva-variable"), "arrays.uint32[1]", value));
  }
  if (status)
  {
    status = _pva_srv->UpdateVariable("sequencer::test::pva-variable");
    (void)ccs::HelperTools::SleepFor(1000000000ul);
  }
  if (status)
  {
    status = variable->GetValue(value);
  }
  if (status)
  {  // Test variable
    status = (1234u
              == ccs::HelperTools::GetAttributeValue<ccs::types::uint32>(
                  _pva_srv->GetVariable("sequencer::test::pva-variable"), "scalars.uint32"));
  }
  if (status)
  {  // Terminate PVA server
    status = Terminate();
  }
  if (status)
  {
    status = (false == variable->GetValue(value));  // Disconnected .. To be reassessed
  }

  ASSERT_EQ(true, status);
}

TEST(PVMonitorVariable, SetValue)
{
  auto variable = sup::sequencer::GlobalVariableRegistry().Create("PVMonitorVariable");
  bool status = static_cast<bool>(variable);

  if (status)
  {
    status = variable->AddAttribute("channel", "sequencer::test::pva-variable");
    variable->Setup();
  }

  ccs::types::AnyValue value("{\"type\":\"uint64\"}");
  if (status)
  {
    status = static_cast<bool>(value.GetType());
  }
  if (status)
  {
    status = (false == variable->SetValue(value));
  }
  ASSERT_EQ(true, status);
}

#undef LOG_ALTERN_SRC
