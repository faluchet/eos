// ----------------------------------------------------------------------
// File: CreateStageRequestModelBuilder.hh
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

#ifndef EOS_CREATESTAGEREQUESTMODELBUILDER_HH
#define EOS_CREATESTAGEREQUESTMODELBUILDER_HH

#include "mgm/Namespace.hh"
#include "mgm/http/rest-api/json/builder/jsoncpp/JsonCppModelBuilder.hh"
#include "mgm/http/rest-api/json/tape/model-builders/validators/TapeJsonCppValidator.hh"
#include "mgm/http/rest-api/model/tape/stage/CreateStageBulkRequestModel.hh"

EOSMGMRESTNAMESPACE_BEGIN

/**
 * Constructs a CreateStageBulkRequestModel out of a JSON string
 */
class CreateStageRequestModelBuilder : public JsonCppModelBuilder<CreateStageBulkRequestModel> {
public:
  CreateStageRequestModelBuilder(const std::string & restApiEndpointId);
  virtual std::unique_ptr<CreateStageBulkRequestModel> buildFromJson(const std::string & json) override;
  inline static const std::string FILES_KEY_NAME = "files";
  inline static const std::string PATH_KEY_NAME = "path";
  inline static const std::string TARGETED_METADATA_KEY_NAME = "targetedMetadata";
protected:
  TapeJsonCppValidatorFactory mValidatorFactory;
  std::string mRestApiEndpointId;
};

EOSMGMRESTNAMESPACE_END

#endif // EOS_CREATESTAGEREQUESTMODELBUILDER_HH
