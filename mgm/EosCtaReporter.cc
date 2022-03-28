//------------------------------------------------------------------------------
// File: EosCtaReporter.cc
// Author: Joao Afonso - CERN
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

#include <functional>

#include "mgm/EosCtaReporter.hh"
#include "mgm/XrdMgmOfs.hh"
#include "mgm/Iostat.hh"

EOSMGMNAMESPACE_BEGIN

void ioStatsWrite(const std::string & in) {
  if(gOFS->IoStats) {
    gOFS->IoStats->WriteRecord(in);
  }
}

EosCtaReporter::EosCtaReporter(std::function<void(const std::string&)> func) {
  writerFunc = func ? func : &ioStatsWrite;
  for (const std::string& key : DEFAULT_PARAMS) {
    params[key] = "";
  }
}

// TODO: Inject output sink as dependency.
void EosCtaReporter::generateEosReportEntry() {
  std::ostringstream oss;
  for(auto it = params.begin(); it != params.end(); it++) {
    if (it != params.begin()) {
      oss << "&";
    }
    oss << it->first << "=" << it->second;
  }
  writerFunc(oss.str());
}

const std::string EosCtaReporter::LOG = "log";
const std::string EosCtaReporter::PATH = "path";
const std::string EosCtaReporter::RUID = "ruid";
const std::string EosCtaReporter::RGID = "rgid";
const std::string EosCtaReporter::TD   = "td";
const std::string EosCtaReporter::HOST = "host";
const std::vector<std::string> EosCtaReporter::DEFAULT_PARAMS = {
        EosCtaReporter::LOG,
        EosCtaReporter::PATH,
        EosCtaReporter::RUID,
        EosCtaReporter::RGID,
        EosCtaReporter::TD,
        EosCtaReporter::HOST
};

// EosCtaReporterPrepareReq, used for prepare request logs

EosCtaReporterPrepareReq::EosCtaReporterPrepareReq(std::function<void(const std::string&)> func) : EosCtaReporter(func) {
  for (const std::string& key : PREP_REQ_DEFAULT_PARAMS) {
    params[key] = "";
  }
}

const std::string EosCtaReporterPrepareReq::PREP_REQ_TS         = "prep.req.ts";
const std::string EosCtaReporterPrepareReq::PREP_REQ_TNS        = "prep.req.tns";
const std::string EosCtaReporterPrepareReq::PREP_REQ_EVENT      = "prep.req.event";
const std::string EosCtaReporterPrepareReq::PREP_REQ_REQID      = "prep.req.reqid";
const std::string EosCtaReporterPrepareReq::PREP_REQ_SENTTOWFE  = "prep.req.senttowfe";
const std::string EosCtaReporterPrepareReq::PREP_REQ_SUCCESSFUL = "prep.req.successful";
const std::string EosCtaReporterPrepareReq::PREP_REQ_ERROR      = "prep.req.error";

const std::vector<std::string> EosCtaReporterPrepareReq::PREP_REQ_DEFAULT_PARAMS = {
        EosCtaReporterPrepareReq::PREP_REQ_TS,
        EosCtaReporterPrepareReq::PREP_REQ_TNS,
        EosCtaReporterPrepareReq::PREP_REQ_EVENT,
        EosCtaReporterPrepareReq::PREP_REQ_REQID,
        EosCtaReporterPrepareReq::PREP_REQ_SENTTOWFE,
        EosCtaReporterPrepareReq::PREP_REQ_SUCCESSFUL,
        EosCtaReporterPrepareReq::PREP_REQ_ERROR
};

// EosCtaReporterPrepareWfe, used for prepare WFE logs

EosCtaReporterPrepareWfe::EosCtaReporterPrepareWfe() {
  for (const std::string& key : PREP_WFE_DEFAULT_PARAMS) {
    params[key] = "";
  }
}

const std::string EosCtaReporterPrepareWfe::PREP_WFE_TS           ="prep.wfe.ts";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_TNS          ="prep.wfe.tns";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_EVENT        ="prep.wfe.event";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_REQID        ="prep.wfe.reqid";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_REQCOUNT     ="prep.wfe.reqcount";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_REQLIST      ="prep.wfe.reqlist";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_EVICTCOUNTER ="prep.wfe.evictcounter";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_ONDISK       ="prep.wfe.ondisk";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_ONTAPE       ="prep.wfe.ontape";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_FIRSTPREPARE ="prep.wfe.firstprepare";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_SENTTOCTA    ="prep.wfe.senttocta";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_ACTIVITY     ="prep.wfe.activity";
const std::string EosCtaReporterPrepareWfe::PREP_WFE_ERROR        ="prep.req.error";

const std::vector<std::string> EosCtaReporterPrepareWfe::PREP_WFE_DEFAULT_PARAMS = {
     EosCtaReporterPrepareWfe::PREP_WFE_TS,
     EosCtaReporterPrepareWfe::PREP_WFE_TNS,
     EosCtaReporterPrepareWfe::PREP_WFE_EVENT,
     EosCtaReporterPrepareWfe::PREP_WFE_REQID,
     EosCtaReporterPrepareWfe::PREP_WFE_REQCOUNT,
     EosCtaReporterPrepareWfe::PREP_WFE_REQLIST,
     EosCtaReporterPrepareWfe::PREP_WFE_EVICTCOUNTER,
     EosCtaReporterPrepareWfe::PREP_WFE_ONDISK,
     EosCtaReporterPrepareWfe::PREP_WFE_ONTAPE,
     EosCtaReporterPrepareWfe::PREP_WFE_FIRSTPREPARE,
     EosCtaReporterPrepareWfe::PREP_WFE_SENTTOCTA,
     EosCtaReporterPrepareWfe::PREP_WFE_ACTIVITY,
     EosCtaReporterPrepareWfe::PREP_WFE_ERROR
};

// EosCtaReporterStagerRm, used for stagerrm logs

EosCtaReporterStagerRm::EosCtaReporterStagerRm() {
  for (const std::string& key : STAGERRM_DEFAULT_PARAMS) {
    params[key] = "";
  }
}

const std::string EosCtaReporterStagerRm::STAGERRM_TS           = "stagerrm.ts";
const std::string EosCtaReporterStagerRm::STAGERRM_TNS          = "stagerrm.tns";
const std::string EosCtaReporterStagerRm::STAGERRM_EVICTCOUNTER = "stagerrm.evictcounter";
const std::string EosCtaReporterStagerRm::STAGERRM_FILEREMOVED  = "stagerrm.fileremoved";
const std::string EosCtaReporterStagerRm::STAGERRM_ERROR        = "stagerrm.error";

const std::vector<std::string> EosCtaReporterStagerRm::STAGERRM_DEFAULT_PARAMS = {
        EosCtaReporterStagerRm::STAGERRM_TS,
        EosCtaReporterStagerRm::STAGERRM_TNS,
        EosCtaReporterStagerRm::STAGERRM_EVICTCOUNTER,
        EosCtaReporterStagerRm::STAGERRM_FILEREMOVED,
        EosCtaReporterStagerRm::STAGERRM_ERROR
};

EOSMGMNAMESPACE_END