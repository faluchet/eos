//------------------------------------------------------------------------------
// File: GroupDrainer.hh
// Author: Abhishek Lekshmanan - CERN
//------------------------------------------------------------------------------

/************************************************************************
 * EOS - the CERN Disk Storage System                                   *
 * Copyright (C) 2022 CERN/Switzerland                                  *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 ************************************************************************/

#pragma once
#include <string_view>
#include "common/AssistedThread.hh"
#include "common/Logging.hh"

namespace eos::mgm {

namespace group_balancer {
class BalancerEngine;
} // group_balancer



class GroupDrainer: public eos::common::LogId {
public:
  GroupDrainer(std::string_view spacename);
  ~GroupDrainer();
  void GroupDrain(ThreadAssistant& assistant) noexcept;
  bool IsUpdateNeeded(bool force=false);
private:
  std::chrono::time_point<std::chrono::steady_clock> mLastUpdated;
  std::chrono::seconds mCacheExpiryTime {300};
  std::string mSpaceName;
  AssistedThread mThread;
  std::unique_ptr<group_balancer::BalancerEngine> mEngine;
};

} // namespace eos::mgm