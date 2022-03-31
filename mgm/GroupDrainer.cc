#include "mgm/GroupDrainer.hh"
#include "mgm/groupbalancer/StdDrainerEngine.hh"
#include "mgm/XrdMgmOfs.hh"
#include "mgm/Master.hh"
#include "mgm/groupbalancer/GroupsInfoFetcher.hh"
#include "common/utils/ContainerUtils.hh"

namespace eos::mgm {

using group_balancer::eosGroupsInfoFetcher;
using group_balancer::GroupStatus;

GroupDrainer::GroupDrainer(std::string_view spacename) : mSpaceName(spacename),
                                                         mEngine(std::make_unique<group_balancer::StdDrainerEngine>()),
                                                         numTx(10000)
{
  mThread.reset(&GroupDrainer::GroupDrain, this);
}

GroupDrainer::~GroupDrainer()
{
  mThread.join();
}

void
GroupDrainer::GroupDrain(ThreadAssistant& assistant) noexcept
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

    pruneTransfers();

    if (isTransfersFull()) {
      // We are currently full, wait for a few seconds before pruning & trying
      // again
      eos_info("msg=\"transfer queue full, pausing before trying again\"");
      assistant.wait_for(std::chrono::seconds(10));
      continue;
    }

    if (isUpdateNeeded(mLastUpdated)) {
      mEngine->populateGroupsInfo(fetcher.fetch());
    }

    if (!mEngine->canPick()) {
      eos_info("msg=\"Cannot pick, Empty source or target groups, check status "
               "if this is not expected\"");
      assistant.wait_for(std::chrono::seconds(10));
      continue;
    }

    prepareTransfers();
  }
}

bool
GroupDrainer::isUpdateNeeded(std::chrono::time_point<std::chrono::steady_clock>& tp,
                             bool force)
{
  using namespace std::chrono_literals;
  auto now = chrono::steady_clock::now();
  if (force) {
    tp = now;
    return true;
  }
  auto elapsed = chrono::duration_cast<chrono::seconds>(now - mLastUpdated);
  if (elapsed > mCacheExpiryTime) {
    tp = now;
    return true;
  }
  return false;
}

// Prune all transfers which are done by Converter, since the converter will
// pop entries off FidTracker once done, this should give us an idea of our
// queued transfers being actually realized
void
GroupDrainer::pruneTransfers()
{
  auto prune_count = eos::common::erase_if(mTransfers, [](const auto& p) {
    return !gOFS->mFidTracker.HasEntry(p.first);
  });

  eos_info("msg=\"pruned %ul transfers, transfers in flight=%ul\"",
           prune_count, mTransfers.size());
}

void
GroupDrainer::prepareTransfers()
{
  uint64_t allowed_tx = numTx - mTransfers.size();
  for (uint64_t i = 0; i < allowed_tx; ++i) {
    prepareTransfer(i);
  }
}

void
GroupDrainer::prepareTransfer(uint64_t index)
{
  auto [grp_drain_from, grp_drain_to] = mEngine->pickGroupsforTransfer(index);

  if (grp_drain_from.empty() || grp_drain_to.empty()) {
    eos_static_info("msg=\"engine gave us empty groups skipping\"");
    return;
  }

  auto fsids = mDrainFsMap.find(grp_drain_from);
  if (fsids == mDrainFsMap.end() || isUpdateNeeded(mDrainMapLastUpdated)) {
    std::tie(fsids, std::ignore) = mDrainFsMap.insert_or_assign(grp_drain_from,
                                                                FsidsinGroup(grp_drain_from));
  }

  auto fsid = eos::common::pickIndexRR(fsids->second, index);
  auto fids = mCacheFileList.find(fsid);
  if (fids == mCacheFileList.end()) {
    populateFids(fsid);
  }

  if (auto fids= mCacheFileList.find(fsid);
      fids != mCacheFileList.end()) {
    if (fids->second.size() > 0) {
      scheduleTransfer(fids->second.back(), grp_drain_from, grp_drain_to);
      fids->second.pop_back();
    }
  } else {
    eos_info("\"msg=couldn't find files in fsid=%d", fsid);
  }
}

//! TODO: implement the following!!!!
void
GroupDrainer::scheduleTransfer(eos::common::FileId::fileid_t fid,
                               std::string_view src_grp,
                               std::string_view tgt_grp)
{
}

void
GroupDrainer::populateFids(eos::common::FileSystem::fsid_t fsid)
{
}

std::vector<eos::common::FileSystem::fsid_t>
FsidsinGroup(std::string_view groupname)
{

  return {};
}
} // eos::mgm
