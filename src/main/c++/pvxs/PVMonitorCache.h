/******************************************************************************
 * $HeadURL: $
 * $Id: $
 *
 * Project       : CODAC Supervision and Automation (SUP) Sequencer component
 *
 * Description   : PVMonitorCache class definition
 *
 * Author        : B.Bauvir (IO)
 *
 * Copyright (c) : 2010-2019 ITER Organization,
 *		  CS 90 046
 *		  13067 St. Paul-lez-Durance Cedex
 *		  France
 *
 * This file is part of ITER CODAC software.
 * For the terms and conditions of redistribution or use of this software
 * refer to the file ITER-LICENSE.TXT located in the top level directory
 * of the distribution package.
 ******************************************************************************/

#ifndef _PVMonitorCache_h_
#define _PVMonitorCache_h_

// Global header files

#include <common/AnyValue.h>
#include <common/BasicTypes.h>  // Misc. type definition
#include <common/PVMonitor.h>

#include <functional>
#include <mutex>

// Local header files

// Constants

// Type definition

namespace sup
{
namespace sequencer
{
class PVMonitorCache : public ccs::base::PVMonitor
{
private:
  /**
   * @brief Intialised flag.
   */
  ccs::types::boolean _initialised = false;

  /**
   * @brief Mutex for concurrent access of Variable.
   */
  mutable std::mutex _async_mutex;

  /**
   * @brief PV monitor copy.
   */
  ccs::types::AnyValue _value;

  /**
   * @brief Callback to call after value was updated.
   */
  std::function<void(const ccs::types::AnyValue&)> _callback;

protected:
public:
  /**
   * @brief Constructor.
   */

  PVMonitorCache(void);

  /**
   * @brief Destructor.
   */

  virtual ~PVMonitorCache(void);

  /**
   * @brief Accessor.
   */

  bool IsInitialised(void) const;
  bool GetValue(ccs::types::AnyValue& value) const;

  bool SetChannel(const ccs::types::char8* const name);
  bool SetCallback(const ccs::types::char8* name,
                   const std::function<void(const ccs::types::AnyValue&)>& cb);

  /**
   * @brief See ccs::base::PVMonitor.
   */

  virtual void HandleEvent(const ccs::base::PVMonitor::Event& event);
  virtual void HandleMonitor(const ccs::types::AnyValue& value);
};

// Global variables

// Function declaration

// Function definition

}  // namespace sequencer

}  // namespace sup

#endif  // _PVMonitorCache_h_
