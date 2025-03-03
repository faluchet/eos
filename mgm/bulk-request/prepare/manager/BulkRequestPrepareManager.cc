//------------------------------------------------------------------------------
//! @file BulkRequestPrepareManager.cc
//! @author Cedric Caffy - CERN
//------------------------------------------------------------------------------

/************************************************************************
 * EOS - the CERN Disk Storage System                                   *
 * Copyright (C) 2017 CERN/Switzerland                                  *
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

#include "BulkRequestPrepareManager.hh"
#include "mgm/bulk-request/BulkRequestFactory.hh"
#include "mgm/bulk-request/exception/PersistencyException.hh"
#include "mgm/bulk-request/exception/BulkRequestException.hh"

EOSBULKNAMESPACE_BEGIN

BulkRequestPrepareManager::BulkRequestPrepareManager(std::unique_ptr<IMgmFileSystemInterface> && mgmFsInterface):
  PrepareManager(std::move(mgmFsInterface)){
}

void BulkRequestPrepareManager::setBulkRequestBusiness(std::shared_ptr<BulkRequestBusiness> bulkRequestBusiness) {
  mBulkRequestBusiness = bulkRequestBusiness;
}

std::unique_ptr<BulkRequest> BulkRequestPrepareManager::getBulkRequest() {
  return std::move(mBulkRequest);
}

void BulkRequestPrepareManager::initializeStagePrepareRequest(XrdOucString& reqid, const common::VirtualIdentity & vid){
  mBulkRequest = std::move(BulkRequestFactory::createStageBulkRequest(vid));
  reqid = mBulkRequest->getId().c_str();
}

void BulkRequestPrepareManager::initializeCancelPrepareRequest(XrdOucString& reqid) {
  mBulkRequest = std::move(BulkRequestFactory::createCancelBulkRequest(reqid.c_str()));
}

void BulkRequestPrepareManager::addFileToBulkRequest(std::unique_ptr<File>&& file){
  if(mBulkRequest != nullptr) {
    mBulkRequest->addFile(std::move(file));
  }
}

void BulkRequestPrepareManager::saveBulkRequest() {
  if(mBulkRequestBusiness != nullptr && mBulkRequest != nullptr){
    try {
      mBulkRequestBusiness->saveBulkRequest(mBulkRequest.get());
    } catch(const PersistencyException & ex){
      eos_err("msg=\"Unable to persist the bulk request %s\" \"ExceptionWhat=%s\"",mBulkRequest->getId().c_str(),ex.what());
      throw ex;
    }
  }
}

const std::shared_ptr<FileCollection::Files> BulkRequestPrepareManager::getFileCollectionFromPersistency(const std::string& reqid) {
  std::shared_ptr<FileCollection::Files> ret(new FileCollection::Files());
  if(mBulkRequestBusiness != nullptr){
    std::unique_ptr<BulkRequest> bulkRequest = mBulkRequestBusiness->getBulkRequest(reqid,BulkRequest::PREPARE_STAGE);
    if(bulkRequest != nullptr){
      ret = bulkRequest->getFiles();
    }
  }
  return ret;
}

bool BulkRequestPrepareManager::ignorePrepareFailures() {
  return true;
}

EOSBULKNAMESPACE_END