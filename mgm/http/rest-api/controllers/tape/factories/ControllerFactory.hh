// ----------------------------------------------------------------------
// File: ControllerFactory.hh
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

#ifndef EOS_CONTROLLERFACTORY_HH
#define EOS_CONTROLLERFACTORY_HH

#include "mgm/Namespace.hh"
#include <string>
#include "mgm/http/rest-api/controllers/Controller.hh"
#include <memory>

EOSMGMRESTNAMESPACE_BEGIN

/**
 * Factory of REST API controllers.
 */
class ControllerFactory {
public:
  static std::unique_ptr<Controller> getStageController(const std::string & accessURL);
  static std::unique_ptr<Controller>
  getArchiveInfoController(const std::string & accessURL);
  static std::unique_ptr<Controller> getReleaseController(const std::string & accessURL);
};

EOSMGMRESTNAMESPACE_END

#endif // EOS_CONTROLLERFACTORY_HH
