/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Gennady Pospelov
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

namespace sup {
namespace sequencer {

const std::string PVClientVariable::Type = "PVClientVariable";

PVClientVariable::PVClientVariable() : Variable(PVClientVariable::Type) {}

bool PVClientVariable::SetupImpl() { return false; }

bool PVClientVariable::GetValueImpl(ccs::types::AnyValue &value) const {
  return false;
}

bool PVClientVariable::SetValueImpl(const ccs::types::AnyValue &value) {
  return false;
}

} // namespace sequencer

} // namespace sup
