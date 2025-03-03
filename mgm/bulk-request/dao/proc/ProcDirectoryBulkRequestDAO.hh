//------------------------------------------------------------------------------
//! @file ProcDirectoryBulkRequestDAO.hh
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

#ifndef EOS_PROCDIRECTORYBULKREQUESTDAO_HH
#define EOS_PROCDIRECTORYBULKREQUESTDAO_HH

#include "mgm/Namespace.hh"
#include "mgm/bulk-request/dao/IBulkRequestDAO.hh"
#include <common/Logging.hh>
#include <mgm/XrdMgmOfs.hh>
#include "mgm/bulk-request/dao/proc/ProcDirectoryBulkRequestLocations.hh"
#include "mgm/bulk-request/dao/proc/ProcDirBulkRequestFile.hh"
#include "mgm/bulk-request/prepare/CancellationBulkRequest.hh"

EOSBULKNAMESPACE_BEGIN

/**
 * This class is the bulk request persistency layer using the eos proc directory
 *
 * The bulk request persistence will be ensured by creating and listing a directory's extended attributes in a the /eos/.../proc/bulkrequest.
 */
class ProcDirectoryBulkRequestDAO : public IBulkRequestDAO, public eos::common::LogId {
public:
  ProcDirectoryBulkRequestDAO(XrdMgmOfs * fileSystem, const ProcDirectoryBulkRequestLocations & mBulkRequestDirSchema);

  /**
   * Save the bulk request by updating the state (to CANCELLED) of a subset of files
   * belonging to a previously submitted stage bulk-request.
   * The files that are contained in the CancellationBulkRequest represent
   * the subset of files that needs to be cancelled on the Stage bulk-request
   * @param bulkRequest the bulkRequest that will be used to update the state
   * of the files located in a previously submitted stage request
   */
  void saveBulkRequest(const CancellationBulkRequest * bulkRequest) override;

  /**
   * Save the bulk request by creating a directory in the /eos/.../proc/ directory and creating one extended attribute
   * per file in this directory:
   * - If a file in the bulk-request exists, the file will be named according to the fileId
   * - If a file in the bulk-request does not exist, the file will be named according to the path provided in the format like the one in the EOS
   * recycle-bin (each '/' will be replaced by "#:#")
   * @param bulkRequest the BulkRequest to save
   */
  void saveBulkRequest(const StageBulkRequest * bulkRequest) override;

  /**
   * Get the bulk-request from the /eos/.../proc directory
   * @param id the id of the bulk-request
   * @param type the type of bulk-request
   * @return the bulk-request associated to the id and the type, nullptr if it does not exist
   */
  std::unique_ptr<BulkRequest> getBulkRequest(const std::string & id, const BulkRequest::Type & type) override;

  /**
   * Delete all the bulk-request of a certain type that were not accessed for  hours
   * @param type the bulk-request type to look for
   * @param seconds the number of seconds after which the bulk-requests can be deleted if they were not queried
   * @returns the number of deleted bulk-request
   */
  uint64_t deleteBulkRequestNotQueriedFor(const BulkRequest::Type & type, const std::chrono::seconds & seconds) override;

  /**
   * Returns true if the bulk-request corresponding to the id and the type
   * passed in parameters exists, false otherwise
   * @param bulkRequestId the id of the bulk-request to check
   * @param type the type of the bulk-request to test
   * @returns true if the bulk-request exists, false otherwise
   */
  bool exists(const std::string & bulkRequestId, const BulkRequest::Type & type) override;

  /**
   * Deletes the bulk-request passed in parameters from the persistency
   * @param bulkRequest the bulk-request to delete
   */
  void deleteBulkRequest(const BulkRequest * bulkRequest) override;

  virtual ~ProcDirectoryBulkRequestDAO(){}

private:
  //Interface to the EOS filesystem to allow the creation of files and directories
  XrdMgmOfs * mFileSystem;
  const ProcDirectoryBulkRequestLocations & mProcDirectoryBulkRequestLocations;
  eos::common::VirtualIdentity mVid;

  const char * ERROR_MSG_XATTR_NAME = "error_msg";
  const char * LAST_ACCESS_TIME_XATTR_NAME = "last_accessed_time";
  inline static const std::string ISSUER_UID_XATTR_NAME = "issuer_uid";
  //File persisted as bulk-request's directory extended attribute will be prefixed by this prefix
  inline static const std::string FILE_ID_XATTR_KEY_PREFIX = "fid.";
  inline static const std::string CREATION_TIME_XATTR_NAME = "creation_time";

  void cancelStageBulkRequest(const CancellationBulkRequest * bulkRequest);

  /**
   * Creates a directory to store the bulk-request files within it
   * @param bulkRequest the bulkRequest to get the id from
   * @param bulkReqProcPath the directory in /proc/ where the files contained in the bulk-request will be saved
   */
  void createBulkRequestDirectory(const BulkRequest * bulkRequest, const std::string & bulkReqProcPath);
  /**
   * Generate the bulk-request directory path within the /eos/.../proc/ directory
   * It is generated according to the id of the bulk-request
   * @param bulkRequest the bulk-request to generate the path from
   * @return the string containing the path of the directory used to store the bulk-request
   */
  std::string generateBulkRequestProcPath(const BulkRequest * bulkRequest);

  /**
   * Generate the bulk-request directory path within the /eos/.../proc/ directory
   * It is generated according to the id of the bulk-request
   * @param bulkRequestId the id of the bulk-request to generate the proc directory path
   * @param type the type of the bulk-request to generate the proc directory path
   * @return the string containing the path of the directory used to store the bulk-request
   */
  std::string generateBulkRequestProcPath(const std::string & bulkRequestId, const BulkRequest::Type & type);

  /**
   * Insert the files contained in the bulk request into the directory created by createBulkRequestDirectory()
   * @param bulkRequest the bulk-request containing the files to insert in the directory
   * @param bulkReqProcPath the he directory in /proc/ where the files contained in the bulk-request will be saved
   */
  //void insertBulkRequestFilesToBulkRequestDirectory(const BulkRequest * bulkRequest, const std::string & bulkReqProcPath);

  /**
   * Performs the cleaning of the bulk-request directory if an exception happens during the persistency of the bulk-request
   * @param bulkReqProcPath the directory where the bulk-request should have been stored
   */
  void cleanAfterExceptionHappenedDuringBulkRequestSave(const std::string & bulkReqProcPath) noexcept;

  /**
   * Deletes the directory located in the path passed in parameter
   * @param path the path of the directory to delete
   */
  void deleteDirectory(const std::string & path);

  /**
   * Returns true if the directory path passed in parameter exists, false otherwise
   * @param dirPath the path of the directory to check its existence
   * @return true if the directory path passed in parameter exists, false otherwise
   */
  bool existsAndIsDirectory(const std::string & dirPath);

  /**
   * creates the bulk request and fills it from the directory passed in parameter
   * @param bulkRequest the bulkRequest to fill
   * @param xattrs the extended attributes of the directory where the bulk-request is persisted
   * @param bulkRequestProcPath the path where the bulk request is persisted
   */
  void fillBulkRequestFromXattrs(bulk::BulkRequest * bulkRequest, const eos::IContainerMD::XAttrMap & xattrs);

  std::unique_ptr<StageBulkRequest> initializeStageBulkRequestFromXattrs(const std::string & requestId, const eos::IContainerMD::XAttrMap & xattrs);
  /**
   * Fills the directoryContent map passed in parameter. The key is the full path of the directory given in parameter
   * the value is the file names that are located in the directory.
   * Reminder: the files that are in the directory of the bulk-request are the fileIds of the files that were submitted with the bulk-request (or
   * transformed paths for the files that were submitted did not exist)
   * @param path the path of the directory to get the content
   * @param directoryContent the map that will be filled with the content of the directory passed in parameter
   */
  void getDirectoryContent(const std::string & path, std::map<std::string, std::set<std::string>> & directoryContent);

  /**
   * Fills the xattrs map with the extended attributes of the file/directory whose path is passed in parameter
   * @param path the path of the file to get the extended attributes from
   * @param xattrs extended attributes map to fill
   */
  void fetchExtendedAttributes(const std::string & path, eos::IContainerMD::XAttrMap & xattrs);

  /**
   * Asynchronously fetch the file metadata by using the eosFileService->getFileMDFut() method. The filesWithFuture map will
   * be filled by this method
   * @param file the file to asynchronously fetch the metadata
   * @param filesWithFuture the map to include the file and its future
   */
  void initiateFileMDFetch(const ProcDirBulkRequestFile & file,std::map<ProcDirBulkRequestFile, folly::Future<IFileMDPtr>> & filesWithFuture);

  /**
   * Wait for the futures associated to the files inserted in the filesWithFuture map (got filled by the initiateFileMDFetch() method).
   * Once all the futures have returned
   * @param filesWithFuture the map containing the files and the associated future to get the metadata from it
   * @param bulkRequest the bulk request in which the files will be added
   */
  void getFilesPathAndAddToBulkRequest(std::map<ProcDirBulkRequestFile, folly::Future<IFileMDPtr>> & filesWithFuture, BulkRequest * bulkRequest);

  /**
   * Sets an extended attribute on the file whose path is passed in parameter
   * @param path the path of the file or directory to set the extended attribute
   * @param xattrName the extended attribute name
   * @param xattrValue the extended attribute value
   */
  void setExtendedAttribute(const std::string & path,const std::string & xattrName, const std::string & xattrValue);

  /**
   * Set the last_access_time extended attribute to the current timestamp on the file/directory passed in parameter
   * @param path the path of the file/directory to set the last acess time
   */
  void updateLastAccessTime(const std::string & path);

  /**
   * Generate the extended attributes map that will be added to the bulk-request directory
   * @param bulkRequest the bulk-request from which the map will be added
   * @param xattrs the map containing the fid of a file associated to an eventual error (e.g prepare submission)
   */
  void generateXattrsMapFromBulkRequest(const BulkRequest * bulkRequest,eos::IContainerMD::XAttrMap & xattrs);

  void generateXattrsMapFromBulkRequest(const StageBulkRequest * bulkRequest, eos::IContainerMD::XAttrMap & xattrs);

  /**
   * Persist the bulk-request in the extended attributes of the directory
   * @param directoryBulkReqPath the directory where each file belonging to the bulk-request will be persisted
   * @param xattrs the map containing each file belonging to the bulk-request
   */
  void persistBulkRequestDirectory(const std::string & directoryBulkReqPath,const eos::IContainerMD::XAttrMap & xattrs);
};

EOSBULKNAMESPACE_END

#endif // EOS_PROCDIRECTORYBULKREQUESTDAO_HH
