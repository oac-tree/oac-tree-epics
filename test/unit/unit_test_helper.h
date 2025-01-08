/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : UserInterface implementation
 *
 * Author        : Walter Van Herck (IO)
 *
 * Copyright (c) : 2010-2025 ITER Organization,
 *                 CS 90 046
 *                 13067 St. Paul-lez-Durance Cedex
 *                 France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
******************************************************************************/

#ifndef SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_
#define SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_

#include <functional>
#include <string>

#include <sup/sequencer/execution_status.h>
#include <sup/sequencer/procedure.h>
#include <sup/sequencer/variable.h>

#include <thread>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

bool WaitForCAChannel(const std::string& channel, const std::string& type_str, double timeout);

class ReadOnlyVariable : public Variable
{
public:
  ReadOnlyVariable(const sup::dto::AnyValue& value);
  ~ReadOnlyVariable();
private:
  sup::dto::AnyValue m_value;
  bool GetValueImpl(sup::dto::AnyValue& value) const override;
  bool SetValueImpl(const sup::dto::AnyValue& value) override;
};

static inline bool TryAndExecute(std::unique_ptr<Procedure>& proc, UserInterface& ui,
                                 const ExecutionStatus& expect = ExecutionStatus::SUCCESS);

static inline bool TryAndExecuteNoReset(std::unique_ptr<Procedure>& proc, UserInterface& ui,
                                        const ExecutionStatus& expect)
{
  bool status = static_cast<bool>(proc);
  proc->Setup();
  if (status)
  {
    ExecutionStatus exec = ExecutionStatus::FAILURE;
    do
    {
      if (exec == ExecutionStatus::RUNNING)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      proc->ExecuteSingle(ui);
      exec = proc->GetStatus();
    } while ((ExecutionStatus::SUCCESS != exec)
             && (ExecutionStatus::FAILURE != exec));
    status = (expect == exec);
  }
  return status;
}

static inline bool TryAndExecute(std::unique_ptr<Procedure>& proc, UserInterface& ui,
                                 const ExecutionStatus& expect)
{
  bool status = TryAndExecuteNoReset(proc, ui, expect);
  if (proc)
  {
    proc->Reset(ui);
  }
  return status;
}

std::string CreateProcedureString(const std::string& body);

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup

#endif // SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_

