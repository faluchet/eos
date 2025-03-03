// ----------------------------------------------------------------------
// File: proc/admin/Space.cc
// Author: Andreas-Joachim Peters - CERN
// ----------------------------------------------------------------------

/************************************************************************
 * EOS - the CERN Disk Storage System                                   *
 * Copyright (C) 2011 CERN/Switzerland                                  *
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

#include "common/Path.hh"
#include "common/Constants.hh"
#include "common/token/EosTok.hh"
#include "mgm/proc/ProcInterface.hh"
#include "mgm/tgc/Constants.hh"
#include "mgm/http/rest-api/Constants.hh"
#include "mgm/http/rest-api/manager/RestApiManager.hh"
#include "mgm/tracker/ReplicationTracker.hh"
#include "mgm/inspector/FileInspector.hh"
#include "mgm/XrdMgmOfs.hh"
#include "mgm/Egroup.hh"
#include "mgm/config/IConfigEngine.hh"
#include "namespace/interface/IChLogFileMDSvc.hh"
#include "namespace/interface/IChLogContainerMDSvc.hh"
#include "namespace/interface/IFsView.hh"
#include "namespace/interface/IContainerMDSvc.hh"
#include "namespace/interface/IView.hh"

EOSMGMNAMESPACE_BEGIN

int
ProcCommand::Space()
{
  if (mSubCmd == "ls") {
    {
      std::string output;
      std::string format;
      std::string mListFormat;
      format = FsView::GetSpaceFormat(std::string(mOutFormat.c_str()));

      if ((mOutFormat == "l")) {
        mListFormat = FsView::GetFileSystemFormat(std::string(mOutFormat.c_str()));
      }

      eos::common::RWMutexReadLock lock(FsView::gFsView.ViewMutex);
      FsView::gFsView.PrintSpaces(output, format, mListFormat, mOutDepth, mSelection);
      stdOut += output.c_str();
    }
  }

  if (mSubCmd == "tracker") {
    std::string output;
    gOFS->mReplicationTracker->Scan(2 * 86400, false, &output);
    stdOut += "# ------------------------------------------------------------------------------------\n";
    stdOut += output.c_str();
    stdOut += "# ------------------------------------------------------------------------------------\n";
    retc = 0;
  }

  if (mSubCmd == "inspector") {
    std::string output;
    std::string options = (pOpaque->Get("mgm.options")) ?
                          pOpaque->Get("mgm.options") : "";
    gOFS->mFileInspector->Dump(output, options);
    stdOut += output.c_str();
    retc = 0;
  }

  if (mSubCmd == "status") {
    bool mformat = (mOutFormat == "m");
    const char* fmtstr = mformat ? "%s=%s " : "%-32s := %s\n";
    std::string space = (pOpaque->Get("mgm.space")) ? pOpaque->Get("mgm.space") :
                        "";
    eos::common::RWMutexReadLock lock(FsView::gFsView.ViewMutex);

    if (FsView::gFsView.mSpaceView.count(space)) {
      if (!mformat) {
        stdOut += "# ------------------------------------------------------------------------------------\n";
      }

      if (!mformat) {
        stdOut += "# Space Variables\n";
      }

      if (!mformat) {
        stdOut += "# ....................................................................................\n";
      }

      std::vector<std::string> keylist;
      FsView::gFsView.mSpaceView[space]->GetConfigKeys(keylist);
      std::sort(keylist.begin(), keylist.end());

      for (size_t i = 0; i < keylist.size(); i++) {
        char line[32678];

        if (((keylist[i] == "nominalsize") ||
             (keylist[i] == "headroom")) && !mformat) {
          XrdOucString sizestring;
          // size printout
          snprintf(line, sizeof(line) - 1, fmtstr, keylist[i].c_str(),
                   eos::common::StringConversion::GetReadableSizeString(sizestring,
                       strtoull(FsView::gFsView.mSpaceView[space]->GetConfigMember(
                                  keylist[i]).c_str(), 0, 10), "B"));
        } else {
          snprintf(line, sizeof(line) - 1, fmtstr, keylist[i].c_str(),
                   FsView::gFsView.mSpaceView[space]->GetConfigMember(keylist[i]).c_str());
        }

        stdOut += line;
      }
    } else {
      stdErr = "error: cannot find space - no space with name=";
      stdErr += space.c_str();
      retc = ENOENT;
    }
  }

  if (mSubCmd == "set") {
    if (pVid->uid == 0) {
      std::string spacename = (pOpaque->Get("mgm.space")) ? pOpaque->Get("mgm.space")
                              : "";
      std::string status = (pOpaque->Get("mgm.space.state")) ?
                           pOpaque->Get("mgm.space.state") : "";

      if ((!spacename.length()) || (!status.length())) {
        stdErr = "error: illegal parameters";
        retc = EINVAL;
      } else {
        eos::common::RWMutexReadLock lock(FsView::gFsView.ViewMutex);

        if (!FsView::gFsView.mSpaceView.count(spacename)) {
          stdErr = "error: no such space - define one using 'space define' or "
                   "add a filesystem under that space!";
          retc = EINVAL;
        } else {
          std::string key = "status";
          {
            // loop over all groups
            std::map<std::string, FsGroup*>::const_iterator it;

            for (it = FsView::gFsView.mGroupView.begin();
                 it != FsView::gFsView.mGroupView.end(); it++) {
              if (!it->second->SetConfigMember(key, status)) {
                stdErr += "error: cannot set status in group <";
                stdErr += it->first.c_str();
                stdErr += ">\n";
                retc = EIO;
              }
            }
          }
          {
            // loop over all nodes
            std::map<std::string, FsNode*>::const_iterator it;

            for (it = FsView::gFsView.mNodeView.begin();
                 it != FsView::gFsView.mNodeView.end(); it++) {
              if (!it->second->SetConfigMember(key, status)) {
                stdErr += "error: cannot set status for node <";
                stdErr += it->first.c_str();
                stdErr += ">\n";
                retc = EIO;
              }
            }
          }
        }
      }
    } else {
      retc = EPERM;
      stdErr = "error: you have to take role 'root' to execute this command";
    }
  }

  if (mSubCmd == "node-set") {
    if (pVid->uid == 0) {
      std::string spacename = (pOpaque->Get("mgm.space")) ? pOpaque->Get("mgm.space")
                              : "";
      std::string key = (pOpaque->Get("mgm.space.node-set.key")) ?
                        pOpaque->Get("mgm.space.node-set.key") : "";
      std::string val = (pOpaque->Get("mgm.space.node-set.val")) ?
                        pOpaque->Get("mgm.space.node-set.val") : "";

      if ((!spacename.length()) || (!key.length()) || (!val.length())) {
        stdErr = "error: illegal parameters";
        retc = EINVAL;
      } else {
        eos::common::RWMutexWriteLock lock(FsView::gFsView.ViewMutex);

        if (!FsView::gFsView.mSpaceView.count(spacename)) {
          stdErr = "error: no such space - define one using 'space define' or "
                   "add a filesystem under that space!";
          retc = EINVAL;
        } else {
          {
            // loop over all nodes
            std::map<std::string, FsNode*>::const_iterator it;

            for (it = FsView::gFsView.mNodeView.begin();
                 it != FsView::gFsView.mNodeView.end(); it++) {
              XrdOucString file = val.c_str();

              if (file.beginswith("file:/")) {
                // load the file on the MGM
                file.erase(0, 5);
                eos::common::Path iPath(file.c_str());
                XrdOucString fpath = iPath.GetPath();

                if (!fpath.beginswith("/var/eos/")) {
                  stdErr = "error: cannot load requested file=";
                  stdErr += file.c_str();
                  stdErr += " - only files under /var/eos/ can bo loaded\n";
                  retc = EINVAL;
                } else {
                  std::ifstream ifs(file.c_str(), std::ios::in | std::ios::binary);

                  if (!ifs) {
                    stdErr = "error: cannot load requested file=";
                    stdErr += file.c_str();
                    retc = EINVAL;
                  } else {
                    val = std::string((std::istreambuf_iterator<char>(ifs)),
                                      std::istreambuf_iterator<char>());
                    // store the value b64 encoded
                    XrdOucString val64;
                    eos::common::SymKey::Base64Encode((char*) val.c_str(), val.length(), val64);
                    val = "base64:";
                    val += val64.c_str();
                    stdOut += "success: loaded contents \n";
                    stdOut += val.c_str();
                  }
                }
              }

              if (!retc && !it->second->SetConfigMember(key, val)) {
                stdErr += "error: cannot set node-set for node <";
                stdErr += it->first.c_str();
                stdErr += ">\n";
                retc = EIO;
              }
            }
          }
        }
      }
    } else {
      retc = EPERM;
      stdErr = "error: you have to take role 'root' to execute this command";
    }
  }

  if (mSubCmd == "node-get") {
    if (pVid->uid == 0) {
      std::string spacename = (pOpaque->Get("mgm.space")) ? pOpaque->Get("mgm.space")
                              : "";
      std::string key = (pOpaque->Get("mgm.space.node-get.key")) ?
                        pOpaque->Get("mgm.space.node-get.key") : "";

      if ((!spacename.length()) || (!key.length())) {
        stdErr = "error: illegal parameters";
        retc = EINVAL;
      } else {
        eos::common::RWMutexReadLock lock(FsView::gFsView.ViewMutex);

        if (!FsView::gFsView.mSpaceView.count(spacename)) {
          stdErr = "error: no such space - define one using 'space define' or "
                   "add a filesystem under that space!";
          retc = EINVAL;
        } else {
          {
            std::string val;
            bool identical = true;
            // loop over all nodes
            std::map<std::string, FsNode*>::const_iterator it;

            for (it = FsView::gFsView.mNodeView.begin();
                 it != FsView::gFsView.mNodeView.end(); it++) {
              std::string new_val = it->second->GetConfigMember(key);

              if (val.length() && new_val != val) {
                identical = false;
              }

              val = new_val;
              stdOut += it->first.c_str();
              stdOut += ":=";
              stdOut += new_val.c_str();
              stdOut += "\n";
            }

            if (identical) {
              stdOut = "*:=";
              stdOut += val.c_str();
              stdOut += "\n";
            }
          }
        }
      }
    } else {
      retc = EPERM;
      stdErr = "error: you have to take role 'root' to execute this command";
    }
  }

  if (mSubCmd == "reset") {
    std::string spacename = (pOpaque->Get("mgm.space")) ? pOpaque->Get("mgm.space")
                            : "";
    XrdOucString option = pOpaque->Get("mgm.option");
    eos::common::RWMutexReadLock fsViewLock(FsView::gFsView.ViewMutex);

    if ((!option.length()) || (option == "drain")) {
      if (FsView::gFsView.mSpaceView.count(spacename)) {
        FsView::gFsView.mSpaceView[spacename]->ResetDraining();
        stdOut = "info: reset draining in space '";
        stdOut += spacename.c_str();
        stdOut += "'";
      } else {
        stdErr = "error: illegal space name";
        retc = EINVAL;
      }
    }

    if ((!option.length()) || (option == "egroup")) {
      gOFS->EgroupRefresh->Reset();
      stdOut += "\ninfo: clear cached EGroup information ...";
    }

    if ((option == "nsfilesystemview")) {
      eos::common::RWMutexWriteLock lock(gOFS->eosViewRWMutex);
      gOFS->eosFsView->shrink();
      stdOut += "\ninfo: resized namespace filesystem view ...";
    }

    if ((option == "nsfilemap")) {
      auto* eos_chlog_filesvc =
        dynamic_cast<eos::IChLogFileMDSvc*>(gOFS->eosFileService);

      if (eos_chlog_filesvc) {
        eos::common::RWMutexWriteLock lock(gOFS->eosViewRWMutex);
        eos_chlog_filesvc->resize();
        stdOut += "\ninfo: resized namespace file map ...";
      } else {
        stdOut += "\n info: ns does not support file map resizing";
      }
    }

    if ((option == "nsdirectorymap")) {
      auto* eos_chlog_dirsvc =
        dynamic_cast<eos::IChLogContainerMDSvc*>(gOFS->eosDirectoryService);

      if (eos_chlog_dirsvc) {
        eos::common::RWMutexWriteLock lock(gOFS->eosViewRWMutex);
        eos_chlog_dirsvc->resize();
        stdOut += "\ninfo: resized namespace directory map ...";
      } else {
        stdOut += "\ninfo: ns does not support directory map resizing";
      }
    }

    if ((option == "ns")) {
      eos::common::RWMutexWriteLock lock(gOFS->eosViewRWMutex);
      gOFS->eosFsView->shrink();
      auto* eos_chlog_filesvc =
        dynamic_cast<eos::IChLogFileMDSvc*>(gOFS->eosFileService);
      auto* eos_chlog_dirsvc =
        dynamic_cast<eos::IChLogContainerMDSvc*>(gOFS->eosDirectoryService);

      if (eos_chlog_filesvc && eos_chlog_dirsvc) {
        eos_chlog_filesvc->resize();
        eos_chlog_dirsvc->resize();
        stdOut += "\ninfo: resized all namespace map ...";
      } else {
        stdOut += "\ninfo: ns does not support map resizing";
      }
    }

    if ((!option.length()) || (option == "mapping")) {
      eos::common::Mapping::Reset();
      stdOut += "\ninfo: clear all user/group uid/gid caches ...\n";
    }

    if (option == "scheduledrain") {
      gOFS->mFidTracker.Clear(TrackerType::Drain);
      stdOut = "info: reset drain scheduling map in space '";
      stdOut += spacename.c_str();
      stdOut += "'";
    }

    if (option == "schedulebalance") {
      gOFS->mFidTracker.Clear(TrackerType::Balance);
      stdOut = "info: reset balance scheduling map in space '";
      stdOut += spacename.c_str();
      stdOut += "'";
    }
  }

  if (mSubCmd == "define") {
    if (pVid->uid == 0) {
      std::string spacename = (pOpaque->Get("mgm.space")) ? pOpaque->Get("mgm.space")
                              : "";
      std::string groupsize = (pOpaque->Get("mgm.space.groupsize")) ?
                              pOpaque->Get("mgm.space.groupsize") : "";
      std::string groupmod = (pOpaque->Get("mgm.space.groupmod")) ?
                             pOpaque->Get("mgm.space.groupmod") : "";
      int gsize = atoi(groupsize.c_str());
      int gmod = atoi(groupmod.c_str());
      char line[1024];
      snprintf(line, sizeof(line) - 1, "%d", gsize);
      std::string sgroupsize = line;
      snprintf(line, sizeof(line) - 1, "%d", gmod);
      std::string sgroupmod = line;

      if ((!spacename.length()) || (!groupsize.length())
          || (groupsize != sgroupsize) || (gsize < 0) || (gsize > 1024)
          || (groupmod != sgroupmod) || (gmod < 0) || (gmod > 1024)) {
        stdErr = "error: illegal parameters";
        retc = EINVAL;

        if ((groupsize != sgroupsize) || (gsize < 0) || (gsize > 1024)) {
          stdErr = "error: <groupsize> must be a positive integer (<=1024)!";
          retc = EINVAL;
        }

        if ((groupmod != sgroupmod) || (gmod < 0) || (gmod > 256)) {
          stdErr = "error: <groupmod> must be a positive integer (<=256)!";
          retc = EINVAL;
        }
      } else {
        eos::common::RWMutexWriteLock lock(FsView::gFsView.ViewMutex);

        if (!FsView::gFsView.mSpaceView.count(spacename)) {
          stdOut = "info: creating space '";
          stdOut += spacename.c_str();
          stdOut += "'";

          if (!FsView::gFsView.RegisterSpace(spacename.c_str())) {
            stdErr = "error: cannot register space <";
            stdErr += spacename.c_str();
            stdErr += ">";
            retc = EIO;
          }
        }

        if (!retc) {
          // set this new space parameters
          if ((!FsView::gFsView.mSpaceView[spacename]->SetConfigMember(
                 std::string("groupsize"), groupsize)) ||
              (!FsView::gFsView.mSpaceView[spacename]->SetConfigMember(
                 std::string("groupmod"), groupmod))) {
            retc = EIO;
            stdErr = "error: cannot set space config value";
          }
        }
      }
    } else {
      retc = EPERM;
      stdErr = "error: you have to take role 'root' to execute this command";
    }
  }

  if (mSubCmd == "config") {
    if (pVid->uid == 0) {
      std::string identifier = (pOpaque->Get("mgm.space.name")) ?
                               pOpaque->Get("mgm.space.name") : "";
      std::string key = (pOpaque->Get("mgm.space.key")) ?
                        pOpaque->Get("mgm.space.key") : "";
      std::string value = (pOpaque->Get("mgm.space.value")) ?
                          pOpaque->Get("mgm.space.value") : "";

      if ((!identifier.length()) || (!key.length()) || (!value.length())) {
        stdErr = "error: illegal parameters";
        retc = EINVAL;
      } else {
        eos::common::RWMutexReadLock lock(FsView::gFsView.ViewMutex);
        FileSystem* fs = nullptr;
        // by host:port name
        std::string path = identifier;

        if (FsView::gFsView.mSpaceView.count(identifier)) {
          if(!strcmp(mgm::rest::TAPE_REST_API_SWITCH_ON_OFF, key.c_str())) {
            //REST API activation
            if ((value != "on") && (value != "off")) {
              retc = EINVAL;
              stdErr = "error: value has to either on or off";
            } else {
              const std::string & spaceName = identifier;
              if(spaceName != "default") {
                retc = EIO;
                stdErr = "error: the tape REST API can only be enabled on the default space";
              } else {
                if (!FsView::gFsView.mSpaceView[identifier]
                         ->SetConfigMember(key, value)) {
                  retc = EIO;
                  stdErr = "error: cannot set space config value";
                } else {
                  if(value == "on") {
                    gOFS->mRestApiManager->getTapeRestApiConfig()->setActivated(true);
                    stdOut += "success: Tape REST API enabled";
                  } else {
                    gOFS->mRestApiManager->getTapeRestApiConfig()->setActivated(false);
                    stdOut += "success: Tape REST API disabled";
                  }
                }
              }
            }
          }
          // set a space related parameter
          if (!key.compare(0, 6, "space.")) {
            key.erase(0, 6);

            if (key.substr(0, 7) == "policy.") {
              if (value == "remove") {
                if (!FsView::gFsView.mSpaceView[identifier]->DeleteConfigMember(key)) {
                  retc = ENOENT;
                  stdErr = "error: key has not been deleted";
                } else {
                  stdOut = "success: removed space policy '";
                  stdOut += key.c_str();
                  stdOut += "'\n";
                }
              }  else {
                // set a space policy parameters e.g. default placement attributes
                if (!FsView::gFsView.mSpaceView[identifier]->SetConfigMember(key, value)) {
                  retc = EIO;
                  stdErr = "error: cannot set space config value";
                } else {
                  stdOut = "success: configured policy in space='";
                  stdOut += identifier.c_str();
                  stdOut += "' as ";
                  stdOut += key.c_str();
                  stdOut += "='";
                  stdOut += value.c_str();
                  stdOut += "'\n";
                  retc = 0 ;
                }
              }
            } else {
              if ((key == "nominalsize") ||
                  (key == "headroom") ||
                  (key == "graceperiod") ||
                  (key == "drainperiod") ||
                  (key == "balancer") ||
                  (key == "balancer.node.rate") ||
                  (key == "balancer.node.ntx") ||
                  (key == "drainer.node.rate") ||
                  (key == "drainer.node.ntx") ||
                  (key == "drainer.node.nfs") ||
                  (key == "drainer.retries") ||
                  (key == "drainer.fs.ntx") ||
                  (key == "converter") ||
                  (key == "tracker") ||
                  (key == "inspector") ||
                  (key == "inspector.interval") ||
                  (key == "lru") ||
                  (key == "lru.interval") ||
                  (key == "wfe") ||
                  (key == "wfe.interval") ||
                  (key == "wfe.ntx") ||
                  (key == "converter.ntx") ||
                  (key == "autorepair") ||
                  (key == "groupbalancer") ||
                  (key == "groupbalancer.ntx") ||
                  (key == "groupbalancer.threshold") ||
                  (key == "groupbalancer.min_file_size") ||
                  (key == "groupbalancer.max_file_size") ||
                  (key == "groupbalancer.engine") ||
                  (key == "groupbalancer.engine.std.threshold") ||
                  (key == "groupbalancer.engine.mm.min_threshold") ||
                  (key == "groupbalancer.engine.mm.max_threshold") ||
                  (key == "geobalancer") ||
                  (key == "geobalancer.ntx") ||
                  (key == "geobalancer.threshold") ||
                  (key == "geo.access.policy.read.exact") ||
                  (key == "geo.access.policy.write.exact") ||
                  (key == "filearchivedgc") ||
                  (key == eos::mgm::tgc::TGC_NAME_QRY_PERIOD_SECS) ||
                  (key == eos::mgm::tgc::TGC_NAME_AVAIL_BYTES) ||
                  (key == eos::mgm::tgc::TGC_NAME_FREE_BYTES_SCRIPT) ||
                  (key == eos::mgm::tgc::TGC_NAME_TOTAL_BYTES) ||
                  (key == "balancer.threshold") ||
                  (key == "token.generation") ||
                  (key == eos::common::SCAN_IO_RATE_NAME) ||
                  (key == eos::common::SCAN_ENTRY_INTERVAL_NAME) ||
                  (key == eos::common::SCAN_DISK_INTERVAL_NAME) ||
                  (key == eos::common::SCAN_NS_INTERVAL_NAME) ||
                  (key == eos::common::SCAN_NS_RATE_NAME)) {
                if ((key == "balancer") || (key == "converter") ||
                    (key == "tracker") || (key == "inspector") ||
                    (key == "autorepair") || (key == "lru") ||
                    (key == "groupbalancer") || (key == "geobalancer") ||
                    (key == "geo.access.policy.read.exact") ||
                    (key == "geo.access.policy.write.exact") ||
                    (key == "filearchivedgc")) {
                  if ((value != "on") && (value != "off")) {
                    retc = EINVAL;
                    stdErr = "error: value has to either on or off";
                  } else {
                    if (!FsView::gFsView.mSpaceView[identifier]->SetConfigMember(key, value)) {
                      retc = EIO;
                      stdErr = "error: cannot set space config value";
                    } else {
                      if (key == "balancer") {
                        if (value == "on") {
                          stdOut += "success: balancer is enabled!";
                        } else {
                          stdOut += "success: balancer is disabled!";
                        }
                      }

                      if (key == "converter") {
                        if (value == "on") {
                          stdOut += "success: converter is enabled!";
                        } else {
                          stdOut += "success: converter is disabled!";
                        }
                      }

                      if (key == "tracker") {
                        if (value == "on") {
                          gOFS->mReplicationTracker->enable();
                          stdOut += "success: tracker is enabled!";
                        } else {
                          gOFS->mReplicationTracker->disable();
                          stdOut += "success: tracker is disabled!";
                        }
                      }

                      if (key == "inspector") {
                        if (value == "on") {
                          gOFS->mFileInspector->enable();
                          stdOut += "success: file inspector is enabled!";
                        } else {
                          gOFS->mFileInspector->disable();
                          stdOut += "success: file inspector is disabled!";
                        }
                      }

                      if (key == "autorepair") {
                        if (value == "on") {
                          stdOut += "success: auto-repair is enabled!";
                        } else {
                          stdOut += "success: auto-repair is disabled!";
                        }
                      }

                      if (key == "groupbalancer") {
                        if (value == "on") {
                          stdOut += "success: groupbalancer is enabled!";
                        } else {
                          stdOut += "success: groupbalancer is disabled!";
                        }
                      }

                      if (key == "geobalancer") {
                        if (value == "on") {
                          stdOut += "success: geobalancer is enabled!";
                        } else {
                          stdOut += "success: geobalancer is disabled!";
                        }
                      }

                      if (key == "geo.access.policy.read.exact") {
                        if (value == "on") {
                          stdOut += "success: geo access policy prefers the exact geo matching replica for reading!";
                        } else {
                          stdOut += "success: geo access policy prefers with a weight the geo matching replica for reading!";
                        }
                      }

                      if (key == "geo.access.policy.write.exact") {
                        if (value == "on") {
                          stdOut += "success: geo access policy prefers the exact geo matching replica for placements!";
                        } else {
                          stdOut += "success: geo access policy prefers with a weight the geo matching replica for placements!";
                        }
                      }

                      if (key == "scheduler.skip.overloaded") {
                        if (value == "on") {
                          stdOut += "success: scheduler skips overloaded eth-out nodes!";
                        } else {
                          stdOut += "success: scheduler does not skip overloaded eth-out nodes!";
                        }
                      }

                      if (key == "filearchivedgc") {
                        if (value == "on") {
                          stdOut += "success: 'file archived' garbage collector is enabled";
                        } else {
                          stdOut += "success: 'file archived' garbage collector is disabled";
                        }
                      }
                    }
                  }
                } else if (key == "wfe") {
                  if ((value != "on") && (value != "off") && (value != "paused")) {
                    retc = EINVAL;
                    stdErr = "error: value has to either on, paused or off";
                  } else {
                    if (!FsView::gFsView.mSpaceView[identifier]->SetConfigMember(key, value)) {
                      retc = EIO;
                      stdErr = "error: cannot set space config value";
                    }
                  }
                } else {
                  errno = 0;
                  unsigned long long size = eos::common::StringConversion::GetSizeFromString(
                                              value.c_str());

                  if (!errno) {
                    if ((key != "balancer.threshold") &&
                        (key != "groupbalancer.threshold") &&
                        (key != "geobalancer.threshold")) {
                      // the threshold is allowed to be decimal!
                      char ssize[1024];
                      snprintf(ssize, sizeof(ssize) - 1, "%llu", size);
                      value = ssize;
                    }

                    if (!FsView::gFsView.mSpaceView[identifier]->SetConfigMember(key, value)) {
                      retc = EIO;
                      stdErr = "error: cannot set space config value";
                    } else {
                      stdOut = "success: setting ";
                      stdOut += key.c_str();
                      stdOut += "=";
                      stdOut += value.c_str();
                    }

                    if ((key == "token.generation")) {
                      eos::common::EosTok::sTokenGeneration = strtoull(value.c_str(), 0, 0);
                    }

                    if ((key == "policy.recycle")) {
                      if (value == "on") {
                        gOFS->enforceRecycleBin = true;
                      } else {
                        gOFS->enforceRecycleBin = false;
                      }
                    }
                  } else {
                    retc = EINVAL;
                    stdErr = "error: value has to be a positiv number";
                  }
                }
              }
            }
          }

          // Set a filesystem related parameter
          if (!key.compare(0, 3, "fs.")) {
            key.erase(0, 3);
            // we disable the autosave, do all the updates and then switch back
            // to autosave and evt. save all changes
            gOFS->ConfEngine->SetAutoSave(false);

            // Store these as a global parameters of the space
            if ((key == "headroom") ||
                (key == "graceperiod") ||
                (key == "drainperiod") ||
                (key == eos::common::SCAN_IO_RATE_NAME) ||
                (key == eos::common::SCAN_ENTRY_INTERVAL_NAME) ||
                (key == eos::common::SCAN_DISK_INTERVAL_NAME) ||
                (key == eos::common::SCAN_NS_INTERVAL_NAME) ||
                (key == eos::common::SCAN_NS_RATE_NAME)) {
              unsigned long long size = eos::common::StringConversion::GetSizeFromString(
                                          value.c_str());
              char ssize[1024];
              snprintf(ssize, sizeof(ssize) - 1, "%llu", size);

              if ((!FsView::gFsView.mSpaceView[identifier]->SetConfigMember(key, ssize))) {
                stdErr += "error: failed to set space parameter <";
                stdErr += key.c_str();
                stdErr += ">\n";
                retc = EINVAL;
              }
            } else {
              if (key != "configstatus") {
                stdErr += "error: not an allowed parameter <";
                stdErr += key.c_str();
                stdErr += ">\n";
                retc = EINVAL;
              }
            }

            for (auto it = FsView::gFsView.mSpaceView[identifier]->begin();
                 it != FsView::gFsView.mSpaceView[identifier]->end(); ++it) {
              fs = FsView::gFsView.mIdView.lookupByID(*it);

              if (fs) {
                // check the allowed strings
                if (((key == "configstatus") &&
                     (eos::common::FileSystem::GetConfigStatusFromString(value.c_str()) !=
                      eos::common::ConfigStatus::kUnknown))) {
                  fs->SetString(key.c_str(), value.c_str());
                  FsView::gFsView.StoreFsConfig(fs);
                } else {
                  errno = 0;
                  eos::common::StringConversion::GetSizeFromString(value.c_str());

                  if (((key == "headroom") || (key == "graceperiod") || (key == "drainperiod") ||
                       (key == eos::common::SCAN_IO_RATE_NAME) ||
                       (key == eos::common::SCAN_ENTRY_INTERVAL_NAME) ||
                       (key == eos::common::SCAN_DISK_INTERVAL_NAME) ||
                       (key == eos::common::SCAN_NS_INTERVAL_NAME) ||
                       (key == eos::common::SCAN_NS_RATE_NAME)) && (!errno)) {
                    fs->SetLongLong(key.c_str(),
                                    eos::common::StringConversion::GetSizeFromString(value.c_str()));
                    FsView::gFsView.StoreFsConfig(fs);
                  } else {
                    stdErr += "error: not an allowed parameter <";
                    stdErr += key.c_str();
                    stdErr += ">\n";
                    retc = EINVAL;
                    break;
                  }
                }
              } else {
                stdErr += "error: cannot identify the filesystem by <";
                stdErr += identifier.c_str();
                stdErr += ">\n";
                retc = EINVAL;
              }
            }

            gOFS->ConfEngine->SetAutoSave(true);
            gOFS->ConfEngine->AutoSave();
          }
        } else {
          retc = EINVAL;
          stdErr = "error: cannot find space <";
          stdErr += identifier.c_str();
          stdErr += ">";
        }
      }
    } else {
      retc = EPERM;
      stdErr = "error: you have to take role 'root' to execute this command";
    }
  }

  if (mSubCmd == "quota") {
    std::string spacename = (pOpaque->Get("mgm.space")) ? pOpaque->Get("mgm.space")
                            : "";
    std::string onoff = (pOpaque->Get("mgm.space.quota")) ?
                        pOpaque->Get("mgm.space.quota") : "";
    std::string key = "quota";

    if (pVid->uid == 0) {
      if ((!spacename.length()) || (!onoff.length()) || ((onoff != "on") &&
          (onoff != "off"))) {
        stdErr = "error: illegal parameters";
        retc = EINVAL;
      } else {
        eos::common::RWMutexReadLock lock(FsView::gFsView.ViewMutex);

        if (FsView::gFsView.mSpaceView.count(spacename)) {
          if (!FsView::gFsView.mSpaceView[spacename]->SetConfigMember(key, onoff)) {
            retc = EIO;
            stdErr = "error: cannot set space config value";
          }
        } else {
          retc = EINVAL;
          stdErr = "error: no such space defined";
        }
      }
    } else {
      retc = EPERM;
      stdErr = "error: you have to take role 'root' to execute this command";
    }
  }

  if (mSubCmd == "rm") {
    if (pVid->uid == 0) {
      std::string spacename = (pOpaque->Get("mgm.space")) ? pOpaque->Get("mgm.space")
                              : "";

      if ((!spacename.length())) {
        stdErr = "error: illegal parameters";
        retc = EINVAL;
      } else {
        eos::common::RWMutexWriteLock lock(FsView::gFsView.ViewMutex);

        if (!FsView::gFsView.mSpaceView.count(spacename)) {
          stdErr = "error: no such space '";
          stdErr += spacename.c_str();
          stdErr += "'";
          retc = ENOENT;
        } else {
          for (auto it = FsView::gFsView.mSpaceView[spacename]->begin();
               it != FsView::gFsView.mSpaceView[spacename]->end(); it++) {
            FileSystem* fs = FsView::gFsView.mIdView.lookupByID(*it);

            if (fs) {
              // check that filesystems are empty
              if ((fs->GetConfigStatus(false) != eos::common::ConfigStatus::kEmpty)) {
                stdErr = "error: unable to remove space '";
                stdErr += spacename.c_str();
                stdErr += "' - filesystems are not all in empty state - try to drain them or: space config <name> configstatus=empty\n";
                retc = EBUSY;
                return SFS_OK;
              }
            }
          }

          common::SharedHashLocator spaceLocator =
            common::SharedHashLocator::makeForSpace(spacename);

          if (!mq::SharedHashWrapper::deleteHash(gOFS->mMessagingRealm.get(),
                                                 spaceLocator)) {
            stdErr = "error: unable to remove config of space '";
            stdErr += spacename.c_str();
            stdErr += "'";
            retc = EIO;
          } else {
            if (FsView::gFsView.UnRegisterSpace(spacename.c_str())) {
              stdOut = "success: removed space '";
              stdOut += spacename.c_str();
              stdOut += "'";
            } else {
              stdErr = "error: unable to unregister space '";
              stdErr += spacename.c_str();
              stdErr += "'";
            }
          }
        }
      }
    } else {
      retc = EPERM;
      stdErr = "error: you have to take role 'root' to execute this command";
    }
  }

  return SFS_OK;
}

EOSMGMNAMESPACE_END
