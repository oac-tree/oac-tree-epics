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

#include <common/RPCServer.h>

#include <SequenceParser.h>

#include <Instruction.h>
#include <InstructionRegistry.h>

// Local header files

#include "SystemCall.h"

#include "NullUserInterface.h"

// Constants

#undef LOG_ALTERN_SRC
#define LOG_ALTERN_SRC "unit-test"

// Type declaration

class SpecialisedRPCHandler : public ::ccs::base::RPCServer
{

  private:

  public:

    SpecialisedRPCHandler (const ccs::types::char8 * const name) : ::ccs::base::RPCServer(name) {};
    virtual ~SpecialisedRPCHandler (void) {};

    virtual ccs::types::AnyValue HandleRequest (const ccs::types::AnyValue& request) {
      log_info("SpecialisedRPCHandler::HandleRequest - Method called");    
      return request; 
    };

};

// Function declaration

// Global variables

static ccs::log::Func_t _log_handler = ccs::log::SetStdout();
static SpecialisedRPCHandler* _rpc_handler = NULL_PTR_CAST(SpecialisedRPCHandler*);

// Function definition

static inline bool Initialise (void)
{

  bool status = (NULL_PTR_CAST(SpecialisedRPCHandler*) == _rpc_handler);

  if (status)
    {
      _rpc_handler = new (std::nothrow) SpecialisedRPCHandler ("sequencer::test::rpc");
      status = (NULL_PTR_CAST(SpecialisedRPCHandler*) != _rpc_handler);
    }

  if (status)
    {
      (void)::ccs::HelperTools::SleepFor(1000000000ul);
    }

  return status;

}

static inline bool Terminate (void)
{

  bool status = (NULL_PTR_CAST(SpecialisedRPCHandler*) != _rpc_handler);

  if (status)
    {
      delete _rpc_handler;
      _rpc_handler = NULL_PTR_CAST(SpecialisedRPCHandler*);
    }

  return status;

}

TEST(RPCClientInstruction, Execute_missing)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("RPCClientInstruction");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::FAILURE == instruction->GetStatus());
    }

  ASSERT_EQ(true, status);

}

TEST(RPCClientInstruction, Execute_undefined)
{

  auto instruction = sup::sequencer::GlobalInstructionRegistry().Create("RPCClientInstruction");

  bool status = static_cast<bool>(instruction);

  if (status)
    {
      status = instruction->AddAttribute("service", "undefined");
    }

  if (status)
    {
      status = (instruction->AddAttribute("datatype", "{\"type\": \"string\"}") && instruction->AddAttribute("instance", "\"undefined\""));
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      instruction->ExecuteSingle(&ui, NULL_PTR_CAST(sup::sequencer::Workspace*));
      status = (sup::sequencer::ExecutionStatus::FAILURE == instruction->GetStatus());
    }

  ASSERT_EQ(true, status);

}

TEST(RPCClientAccessInstruction, Execute_success) // Must be associated to a variable in the workspace
{

  auto proc = sup::sequencer::ParseProcedureString(
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Procedure xmlns=\"http://codac.iter.org/sup/sequencer\" version=\"1.0\"\n"
    "           name=\"Trivial procedure for testing purposes\"\n"
    "           xmlns:xs=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    "           xs:schemaLocation=\"http://codac.iter.org/sup/sequencer sequencer.xsd\">\n"
    "    <Sequence>\n"
    "        <RPCClientInstruction name=\"rpc-client\"\n"
    "            service=\"sequencer::test::rpc\"\n"
    "            request=\"request\" reply=\"reply\"/>\n"
    "        <LogTrace input=\"request\"/>\n"
    "        <LogTrace input=\"reply\"/>\n"
    "    </Sequence>\n"
    "    <Workspace>\n"
    "        <Local name=\"request\" type='{\"type\":\"Request_t\",\"attributes\":[{\"value\":{\"type\":\"uint32\"}}]}' value='{\"value\":1234u}'/>\n"
    "        <FileVariable name=\"reply\" file=\"/tmp/file-variable-struct.dat\"/>\n"
    "    </Workspace>\n"
    "</Procedure>");

  bool status = static_cast<bool>(proc);

  if (status)
    {
      status = Initialise();
    }

  if (status)
    {
      sup::sequencer::gtest::NullUserInterface ui;
      sup::sequencer::ExecutionStatus exec = sup::sequencer::ExecutionStatus::FAILURE;

      do
        {
          (void)ccs::HelperTools::SleepFor(100000000ul); // Let system breathe
          proc->ExecuteSingle(&ui);
          exec = proc->GetStatus();
        }
      while ((sup::sequencer::ExecutionStatus::SUCCESS != exec) &&
             (sup::sequencer::ExecutionStatus::FAILURE != exec));

      status = (sup::sequencer::ExecutionStatus::SUCCESS == exec);
    }

  // Test variable

  if (status)
    {
      ccs::types::AnyValue value; 
      status = ::ccs::HelperTools::ReadFromFile(&value, "/tmp/file-variable-struct.dat");

      if (status)
	{ // Ignore structure .. just test payload
	  status = (1234u == static_cast<ccs::types::uint32>(value));
	}
    }

  // Remove temp. files
  (void)::ccs::HelperTools::ExecuteSystemCall("/usr/bin/rm -rf /tmp/file-variable-struct.dat");

  (void)Terminate();

  ASSERT_EQ(true, status);

}

#undef LOG_ALTERN_SRC
