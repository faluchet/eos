#include "mgm/GroupDrainer.hh"
#include "mgm/groupbalancer/StdDrainerEngine.hh"
#include "mgm/XrdMgmOfs.hh"
#include "mgm/Master.hh"
#include "mgm/groupbalancer/GroupsInfoFetcher.hh"

namespace eos::mgm {

using group_balancer::eosGroupsInfoFetcher;
using group_balancer::GroupStatus;

GroupDrainer::GroupDrainer(std::string_view spacename) : mSpaceName(spacename),
                                                         mEngine(std::make_unique<group_balancer::StdDrainerEngine>())

{
  mThread.reset(&GroupDrainer::GroupDrain, this);
}

GroupDrainer::~GroupDrainer()
{
  mThread.join();
}

void
GroupDrainer::GroupDrain(ThreadAssistant& assistant)
{
  eosGroupsInfoFetcher fetcher(mSpaceName,
                               [](GroupStatus s) {
                                 return s == GroupStatus::DRAIN || s == GroupStatus::ON;
                               });

  while (!assistant.terminationRequested()) {
    if (!gOFS->mMaster->IsMaster()) {
      assistant.wait_for(std::chrono::seconds(60));
      eos_debug("%s", "msg=\"GroupDrainer disabled for slave\"");
      continue;
    }

    if (IsUpdateNeeded()) {
      mEngine->populateGroupsInfo(fetcher.fetch());
    }

  }
}
bool
GroupDrainer::IsUpdateNeeded(bool force)
{
  using namespace std::chrono_literals;
  auto now = chrono::steady_clock::now();
  if (force) {
    mLastUpdated = now;
    return true;
  }
  auto elapsed = chrono::duration_cast<chrono::seconds>(now - mLastUpdated);
  if (elapsed > mCacheExpiryTime) {
    mLastUpdated = now;
    return true;
  }
  return false;
}

} // eos::mgm
