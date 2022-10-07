/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : SUP - Sequencer
 *
 * Description   : Sequencer for operational procedures
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

#ifndef _SUP_SEQUENCER_PLUGIN_EPICS_LOG_INSTRUCTION_H_
#define _SUP_SEQUENCER_PLUGIN_EPICS_LOG_INSTRUCTION_H_

#include <sup/sequencer/variable.h>

namespace sup
{
namespace sequencer
{

/**
 * @brief SystemClockVariable class.
 *
 * @details Returns current time when its value is retrieved.
 *
 * @todo Assess if different format should be supported, e.g. ISO8601 is the passed value
 * is string, etc.
 */
class SystemClockVariable : public Variable
{
public:
  SystemClockVariable();
  ~SystemClockVariable() override;

  static const std::string Type;

private:
  bool GetValueImpl(sup::dto::AnyValue& value) const override;
  bool SetValueImpl(const sup::dto::AnyValue& value) override;
  bool SetupImpl(const sup::dto::AnyTypeRegistry& registry) override;
  void ResetImpl() override;
  sup::dto::AnyType m_time_type;
};

}  // namespace sequencer

}  // namespace sup

#endif  // _SUP_SEQUENCER_PLUGIN_EPICS_LOG_INSTRUCTION_H_
