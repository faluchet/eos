// ----------------------------------------------------------------------
// File: PathsModel.hh
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

#ifndef EOS_PATHSMODEL_HH
#define EOS_PATHSMODEL_HH

#include "mgm/Namespace.hh"
#include <vector>
#include <string>
#include "mgm/http/rest-api/model/tape/common/FilesContainer.hh"

EOSMGMRESTNAMESPACE_BEGIN

/**
 * This object represents an array of paths.
 * The client would use this object for archiveinfo or release.
 */
class PathsModel {
public:
  PathsModel() = default;
  void addFile(const std::string & path);
  const FilesContainer & getFiles() const;
private:
  FilesContainer mFilesContainer;
};

EOSMGMRESTNAMESPACE_END

#endif // EOS_PATHSMODEL_HH
