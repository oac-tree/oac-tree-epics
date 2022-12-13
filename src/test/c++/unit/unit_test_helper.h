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
 * Copyright (c) : 2010-2022 ITER Organization,
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

#include <sup/sequencer/variable.h>

namespace sup {

namespace sequencer {

namespace unit_test_helper {

bool BusyWaitFor(double timeout_sec, std::function<bool()> predicate);

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

} // namespace unit_test_helper

} // namespace sequencer

} // namespace sup

#endif // SUP_SEQUENCER_PLUGIN_EPICS_UNIT_TEST_HELPER_H_

