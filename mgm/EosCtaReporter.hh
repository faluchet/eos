//------------------------------------------------------------------------------
// File: EosCtaReporter.hh
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

#pragma once

#include <vector>
#include <map>
#include <string>
#include <functional>

#include "mgm/Namespace.hh"

EOSMGMNAMESPACE_BEGIN

class EosCtaReporter {
public:

  static const std::string LOG;
  static const std::string PATH;
  static const std::string RUID;
  static const std::string RGID;
  static const std::string TD;
  static const std::string HOST;

  EosCtaReporter(std::function<void(const std::string&)> func= nullptr);
  EosCtaReporter(const EosCtaReporter &c) = delete;
  EosCtaReporter(EosCtaReporter &&c) {
    *this = std::move(c);
  }
  EosCtaReporter & operator=(const EosCtaReporter&) = delete;
  EosCtaReporter & operator=(EosCtaReporter &&c) {
    params = std::move(c.params);
    writerFunc = c.writerFunc;
    c.writeBack = false;
  }

  virtual ~EosCtaReporter() {
    if(writeBack) {
      generateEosReportEntry();
    }
  }

  template<typename T>
  void addParam(std::string key, const T& val) {
    params[key] = std::to_string(val);
  }

  void addParam(std::string key, const std::string& val) {
    params[key] = val;
  }

  void addParam(std::string key, const bool& val) {
    params[key] = (val ? "true" : "false");
  }

  void addParam(std::string key, const char* val) {
    params[key] = val;
  }

protected:
  static const std::vector<std::string> DEFAULT_PARAMS;
  std::map<std::string, std::string> params;
  bool writeBack = true;

private:
  // It will be called during destruction of EosCtaReporter
  // Do not call explicitly
  void generateEosReportEntry();
  std::function<void(const std::string&)> writerFunc;
};

class EosCtaReporterPrepareReq : public EosCtaReporter {
public:
  static const std::string PREP_REQ_TS;
  static const std::string PREP_REQ_TNS;
  static const std::string PREP_REQ_EVENT;
  static const std::string PREP_REQ_REQID;
  static const std::string PREP_REQ_SENTTOWFE;
  static const std::string PREP_REQ_SUCCESSFUL;
  static const std::string PREP_REQ_ERROR;
  EosCtaReporterPrepareReq(std::function<void(const std::string&)> func);
protected:
  static const std::vector<std::string> PREP_REQ_DEFAULT_PARAMS;
};

class EosCtaReporterPrepareWfe : public EosCtaReporter {
public:
  static const std::string PREP_WFE_TS;
  static const std::string PREP_WFE_TNS;
  static const std::string PREP_WFE_EVENT;
  static const std::string PREP_WFE_REQID;
  static const std::string PREP_WFE_REQCOUNT;
  static const std::string PREP_WFE_REQLIST;
  static const std::string PREP_WFE_EVICTCOUNTER;
  static const std::string PREP_WFE_ONDISK;
  static const std::string PREP_WFE_ONTAPE;
  static const std::string PREP_WFE_FIRSTPREPARE;
  static const std::string PREP_WFE_SENTTOCTA;
  static const std::string PREP_WFE_ACTIVITY;
  static const std::string PREP_WFE_ERROR;
  EosCtaReporterPrepareWfe();
protected:
  static const std::vector<std::string> PREP_WFE_DEFAULT_PARAMS;
};

class EosCtaReporterStagerRm : public EosCtaReporter {
public:
  static const std::string STAGERRM_TS;
  static const std::string STAGERRM_TNS;
  static const std::string STAGERRM_EVICTCOUNTER;
  static const std::string STAGERRM_FILEREMOVED;
  static const std::string STAGERRM_ERROR;
  EosCtaReporterStagerRm();
protected:
  static const std::vector<std::string> STAGERRM_DEFAULT_PARAMS;
};

EOSMGMNAMESPACE_END