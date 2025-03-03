//------------------------------------------------------------------------------
//! @file BulkRequestHelper.hh
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

#ifndef EOS_BULKREQUESTHELPER_HH
#define EOS_BULKREQUESTHELPER_HH

#include "mgm/Namespace.hh"
#include "common/StringConversion.hh"

EOSBULKNAMESPACE_BEGIN

class BulkRequestHelper {
public:
  /**
   * Allows to generate a bulk-request ID
   * @return the id for a new BulkRequest
   */
  static std::string generateBulkRequestId () {
    return common::StringConversion::timebased_uuidstring();
  }
};

EOSBULKNAMESPACE_END

#endif // EOS_BULKREQUESTHELPER_HH
