// ----------------------------------------------------------------------
// File: Action.hh
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

#ifndef EOS_TAPEACTION_HH
#define EOS_TAPEACTION_HH

#include "mgm/Namespace.hh"
#include "mgm/http/rest-api/action/Action.hh"
#include "mgm/http/rest-api/business/tape/ITapeRestApiBusiness.hh"
#include "mgm/http/rest-api/response/tape/factories/TapeRestApiResponseFactory.hh"

EOSMGMRESTNAMESPACE_BEGIN

class TapeAction : public Action {
public:
  TapeAction(const std::string & urlPattern,const common::HttpHandler::Methods method, std::shared_ptr<ITapeRestApiBusiness> tapeRestApiBusiness):
    Action(urlPattern,method),mTapeRestApiBusiness(tapeRestApiBusiness){}
  virtual common::HttpResponse * run(common::HttpRequest * request, const common::VirtualIdentity * vid) override = 0;
protected:
  std::shared_ptr<ITapeRestApiBusiness> mTapeRestApiBusiness;
  TapeRestApiResponseFactory mResponseFactory;
};

EOSMGMRESTNAMESPACE_END

#endif // EOS_TAPEACTION_HH
