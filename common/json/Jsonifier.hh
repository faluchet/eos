// ----------------------------------------------------------------------
// File: Jsonifier.hh
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
#ifndef EOS_JSONIFIER_HH
#define EOS_JSONIFIER_HH

#include "common/Namespace.hh"
#include <sstream>

EOSCOMMONNAMESPACE_BEGIN

/**
 * Inherit this interface in order to implement
 * the way to generate the json representation of any object
 * @tparam Obj the Object you want to generate the json representation from
 */
template<typename Obj>
class Jsonifier {
public:
  /**
   * Implement this method to generate the json representation of any object
   * @param object the object from which you want the json representation
   * @param ss the stream where this json representation will be pushed to
   */
  virtual void jsonify(const Obj * object,std::stringstream & ss) = 0;
};

EOSCOMMONNAMESPACE_END

#endif // EOS_JSONIFIER_HH
