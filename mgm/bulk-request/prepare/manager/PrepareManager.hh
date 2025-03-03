//------------------------------------------------------------------------------
//! @file PrepareManager.hh
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
#ifndef EOS_PREPAREMANAGER_HH
#define EOS_PREPAREMANAGER_HH

#include "common/Logging.hh"
#include "mgm/Namespace.hh"
#include <XrdSfs/XrdSfsInterface.hh>
#include <list>
#include <string>
#include "mgm/XrdMgmOfs.hh"
#include "mgm/bulk-request/interface/IMgmFileSystemInterface.hh"
#include "mgm/bulk-request/prepare/query-prepare/QueryPrepareResult.hh"
#include <mgm/bulk-request/FileCollection.hh>
#include "mgm/bulk-request/BulkRequest.hh"

EOSBULKNAMESPACE_BEGIN

/**
 * This class manages all the operations linked to the preparation of a file:
 * - queue it for retrieval on the tape system
 * - query the preparation
 */
class PrepareManager : public eos::common::LogId {
public:
  /**
   * Types of prepare action
   */
  enum PrepareAction {
    STAGE,
    EVICT,
    ABORT
  };
  /**
   * Constructor
   * @param pargs Xrootd prepare arguments
   * @param error Xrootd error information to fill if there are any errors
   * @param client the client who issued the prepare
   */
  PrepareManager(std::unique_ptr<IMgmFileSystemInterface> && mgmFsInterface);


  /**
   * Allows to launch a prepare logic on the files passed in parameter
   * @param pargs Xrootd prepare arguments (containing the path of the files)
   * @param error Xrootd error information to fill if there are any errors
   * @param client the client who issued the prepare
   * @returns the status code of the issued prepare request
   */
  virtual int prepare(XrdSfsPrep &pargs, XrdOucErrInfo & error, const XrdSecEntity* client) noexcept;

  /**
   * Allows to launch a prepare logic on the files passed in parameter. Will not perform a client map
   * as the vid is already given
   * @param pargs Xrootd prepare arguments (containing the path of the files)
   * @param error Xrootd error information to fill if there are any errors
   * @param vid the vid of the client who issued the prepare
   * @return the status code of the issued prepare request
   */
  virtual int prepare(XrdSfsPrep &pargs, XrdOucErrInfo & error, const common::VirtualIdentity * vid) noexcept;

  /**
   * Allows to launch a query prepare logic on the files passed in parameter
   * @param pargs Xrootd prepare arguments (containing the path of the files)
   * @param error Xrootd error information to fill if there are any errors
   * @param client the client who issued the query prepare
   * @returns the query prepare result object containing the result of the query prepare request
   */
  virtual std::unique_ptr<QueryPrepareResult> queryPrepare(XrdSfsPrep &pargs, XrdOucErrInfo & error, const XrdSecEntity* client);

  /**
   * Allows to launch a query prepare logic on the files passed in parameter
   * @param pargs Xrootd prepare arguments (containing the path of the files)
   * @param error Xrootd error information to fill if there are any errors
   * @param vid the vid of the client who issued the query prepare
   * @returns the query prepare result object containing the result of the query prepare request
   */
  virtual std::unique_ptr<QueryPrepareResult> queryPrepare(XrdSfsPrep &pargs,XrdOucErrInfo & error, const common::VirtualIdentity * vid);

  virtual ~PrepareManager(){}

protected:

  virtual void initializeStagePrepareRequest(XrdOucString & reqid, const common::VirtualIdentity & vid);

  virtual void initializeCancelPrepareRequest(XrdOucString & reqid);

  virtual bool ignorePrepareFailures();

  virtual void setErrorToBulkRequest(const std::string & path, const std::string & error) {}

  /**
   * Returns the Prepare actions to perform from the options given by Xrootd (XrdSfsPrep.opts)
   * @param pargsOpts the prepare options given by Xrootd (XrdSfsPrep.opts)
   * @return the Prepare actions to perform from the options given by Xrootd (XrdSfsPrep.opts)
   */
  const int getPrepareActionsFromOpts(const int pargsOpts) const;

  /**
   * @return true if this prepare request is a stage one, false otherwise
   */
  virtual const bool isStagePrepare() const;

  /**
   * Triggers the prepare workflow to all the pathsToPrepare passed in parameter
   * @param pathsToPrepare the paths of the files on which we want to trigger a prepare workflow
   * @param cmd the command to run in the Workflow engine
   * @param event the event to trigger (sync::prepare, sync::evict_prepare...)
   * @param reqid the requestId of this prepare request
   * @param error The error that will be returned to the client if an error happens
   * @param vid the identity of the person who issued the prepare request
   */
  void triggerPrepareWorkflow(const std::list<std::pair<char**, char**>> & pathsToPrepare, const std::string & cmd, const std::string &event, const XrdOucString & reqid, XrdOucErrInfo & error, const eos::common::VirtualIdentity& vid);

  /**
   * Will call the business layer to persist the bulk request
   */
  virtual void saveBulkRequest();

  /**
   * Will add the prepared path to the bulk request if it exists
   * @param file the file to add to the bulk-request
   */
  virtual void addFileToBulkRequest(std::unique_ptr<File> && file);

  /**
   * Returns the files that were persisted if the id corresponds to an already persisted stage bulk-request
   * @param reqid the request id of the already submitted prepare stage request
   * @return the collection of the files that were submitted for staging (bulk-request). Here nothing will be returned in the collection as the persistency
   * is accessible only via the BulkRequestPrepareManager.
   */
  virtual const std::shared_ptr<FileCollection::Files> getFileCollectionFromPersistency(const std::string & reqid);

  /**
   * Perform the prepare logic
   * @param pargs Xrootd prepare arguments
   * @param error Xrootd error information to fill if there are any errors
   * @param client the client who issued the prepare
   * @param vid the vid of the client if the latter has already been mapped. (Avoids an IdMap call on the client param)
   * @returns the status code of the issued prepare request
   */
  int doPrepare(XrdSfsPrep &pargs, XrdOucErrInfo & error, const XrdSecEntity* client, const common::VirtualIdentity * vidClient = nullptr) noexcept;

  /**
   * Perform the query prepare logic
   * @param pargs Xrootd prepare arguments
   * @param error Xrootd error information to fill if there are any errors
   * @param client the client who issued the query prepare
   * @param vid the vid of the client if the latter has already been mapped. (Avoids an IdMap call on the client param)
   * @returns the status code of the issued prepare request
   */
  int doQueryPrepare(XrdSfsPrep &pargs, XrdOucErrInfo & error, const XrdSecEntity* client, QueryPrepareResult & result, const common::VirtualIdentity * vidClient = nullptr);

  inline static std::string mEpname="prepare";
  //The prepare action that is launched by the "prepare()" method
  PrepareAction mPrepareAction;
  //MGM file system interface
  std::unique_ptr<IMgmFileSystemInterface> mMgmFsInterface;
};

EOSBULKNAMESPACE_END

#endif // EOS_PREPAREMANAGER_HH
