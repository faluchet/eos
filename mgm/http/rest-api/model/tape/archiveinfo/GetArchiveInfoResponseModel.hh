// ----------------------------------------------------------------------
// File: GetArchiveInfoResponseModel.hh
// Author: Cedric Caffy - CERN
// ----------------------------------------------------------------------

/************************************************************************
 * EOS - the CERN Disk Storage System                                   *
 * Copyright (C) 2013 CERN/Switzerland                                  *
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
#ifndef EOS_GETARCHIVEINFORESPONSEMODEL_HH
#define EOS_GETARCHIVEINFORESPONSEMODEL_HH

#include "mgm/Namespace.hh"
#include "common/json/Jsonifiable.hh"
#include "mgm/bulk-request/response/QueryPrepareResponse.hh"

EOSMGMRESTNAMESPACE_BEGIN

/**
 * This Model represents the object that will be returned to the client
 * for any ArchiveInfo request
 */
class GetArchiveInfoResponseModel
    : public common::Jsonifiable<GetArchiveInfoResponseModel> {
public:
  GetArchiveInfoResponseModel(std::shared_ptr<bulk::QueryPrepareResponse> queryPrepareResponse):mQueryPrepareResponse(queryPrepareResponse){}
  std::shared_ptr<bulk::QueryPrepareResponse> getQueryPrepareResponse() const;
private:
  std::shared_ptr<bulk::QueryPrepareResponse> mQueryPrepareResponse;
};

EOSMGMRESTNAMESPACE_END

#endif // EOS_GETARCHIVEINFORESPONSEMODEL_HH
