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

#include <SequenceParser.h>
#include <Variable.h>
#include <VariableRegistry.h>

// Local header files

#include "NullUserInterface.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "unit-test"

static ccs::log::Func_t _log_handler = ccs::log::SetStdout();

static const std::string PVACCESSWRITEPROCEDURE = R"RAW(<?xml version="1.0" encoding="UTF-8"?>
<Procedure xmlns="http://codac.iter.org/sup/sequencer" version="1.0"
           name="Trivial procedure for testing purposes"
           xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
           xs:schemaLocation="http://codac.iter.org/sup/sequencer sequencer.xsd">
    <Repeat maxCount="3">
        <Sequence>
            <Wait name="wait"
                timeout="0.2"/>
            <Copy input="curr-time" output="pvxs-variable.timestamp"/>
            <Copy input="curr-time" output="pvxs-other.timestamp"/>
            <LogTrace name="variable"
                input="pvxs-variable"/>
            <LogTrace name="other"
                input="pvxs-other"/>
        </Sequence>
    </Repeat>
    <Workspace>
        <SystemClock name="curr-time"/>
        <PVServerVariable name="pvxs-variable"
            channel="seq::test::variable"
            datatype='{"type":"seq::test::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'
            instance='{"timestamp":0,"value":0.1}'/>
        <PVServerVariable name="pvxs-other"
            channel="seq::test::another"
            datatype='{"type":"seq::test::Type/v1.0","attributes":[{"timestamp":{"type":"uint64"}},{"value":{"type":"float32"}}]}'
            instance='{"timestamp":0,"value":1.0}'/>
    </Workspace>
</Procedure>)RAW";

TEST(PVServerVariable, ProcedureVariable)
{
  sup::sequencer::gtest::NullUserInterface ui;
  auto proc = sup::sequencer::ParseProcedureString(PVACCESSWRITEPROCEDURE);
  ASSERT_TRUE(static_cast<bool>(proc));
  ASSERT_TRUE(proc->Setup());

  sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

  do
  {
    (void)ccs::HelperTools::SleepFor(100000000ul); // Let system breathe
    proc->ExecuteSingle(&ui);
    exec = proc->GetStatus();
  } while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
           (sup::sequencer::ExecutionStatus::FAILURE != exec));

  EXPECT_EQ(exec, sup::sequencer::ExecutionStatus::SUCCESS);
}

#undef LOG_ALTERN_SRC
