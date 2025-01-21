/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) oac-tree component
 *
 * Description   : Variable plugin implementation
 *
 * Author        : Gennady Pospelov
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

#ifndef SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_SHARED_SERVER_H_
#define SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_SHARED_SERVER_H_

#include <sup/dto/anyvalue.h>

#include <map>
#include <memory>

namespace sup
{
namespace epics
{
class PvAccessServer;
}  // namespace epics

namespace oac_tree
{
class PvAccessSharedServer
{
public:
  using VariableCallback = std::function<void(const sup::dto::AnyValue& val)>;

  PvAccessSharedServer();
  ~PvAccessSharedServer();

  void AddVariable(const std::string& name, const sup::dto::AnyValue& start_val,
                   VariableCallback cb);

  sup::dto::AnyValue GetValue(const std::string& name);

  bool SetValue(const std::string& name, const sup::dto::AnyValue& value);

  void Setup();

  void Teardown();

private:
  void EnsureServer();
  void DelegateCallbacks(const std::string&, const sup::dto::AnyValue& value);
  std::unique_ptr<epics::PvAccessServer> m_server;
  std::map<std::string, VariableCallback> m_var_callbacks;
};

}  // namespace oac_tree

}  // namespace sup

#endif  // SUP_OAC_TREE_PLUGIN_EPICS_PV_ACCESS_SHARED_SERVER_H_
