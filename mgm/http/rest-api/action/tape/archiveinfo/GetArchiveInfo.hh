// ----------------------------------------------------------------------
// File: GetArchiveInfo.hh
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

#ifndef EOS_GETARCHIVEINFO_HH
#define EOS_GETARCHIVEINFO_HH

#include "mgm/Namespace.hh"
#include "mgm/http/rest-api/action/tape/TapeAction.hh"
#include "mgm/http/rest-api/json/builder/JsonModelBuilder.hh"
#include "mgm/http/rest-api/json/tape/TapeRestApiJsonifier.hh"
#include "mgm/http/rest-api/model/tape/archiveinfo/GetArchiveInfoResponseModel.hh"

EOSMGMRESTNAMESPACE_BEGIN

class GetArchiveInfo : public TapeAction {
public:
  GetArchiveInfo(const std::string & accessURL,const common::HttpHandler::Methods method,std::shared_ptr<ITapeRestApiBusiness> tapeRestApiBusiness,std::shared_ptr<JsonModelBuilder<PathsModel>> inputJsonModelBuilder,std::shared_ptr<TapeRestApiJsonifier<GetArchiveInfoResponseModel>> outputObjectJsonifier):
    TapeAction(accessURL,method,tapeRestApiBusiness),mInputJsonModelBuilder(inputJsonModelBuilder),mOutputObjectJsonifier(outputObjectJsonifier){}
  common::HttpResponse * run(common::HttpRequest * request, const common::VirtualIdentity * vid) override;
private:
  std::shared_ptr<JsonModelBuilder<PathsModel>> mInputJsonModelBuilder;
  std::shared_ptr<TapeRestApiJsonifier<GetArchiveInfoResponseModel>>  mOutputObjectJsonifier;
};

EOSMGMRESTNAMESPACE_END

#endif // EOS_GETARCHIVEINFO_HH
