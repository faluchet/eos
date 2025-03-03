//------------------------------------------------------------------------------
//! @file RestApiTest.hh
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

#ifndef EOS_RESTAPITEST_HH
#define EOS_RESTAPITEST_HH

#include "mgm/Namespace.hh"
#include "mgm/http/rest-api/handler/RestHandler.hh"
#include "mgm/http/rest-api/config/tape/TapeRestApiConfig.hh"
#include <gtest/gtest.h>

USE_EOSMGMRESTNAMESPACE;

class RestApiTest : public ::testing::Test {
protected:
  std::unique_ptr<TapeRestApiConfig> mDefaultConfig;

  RestApiTest():testing::Test(),mDefaultConfig(std::make_unique<TapeRestApiConfig>()){}

  virtual void SetUp() {

  }

  virtual void TearDown() {

  }

  static std::unique_ptr<eos::common::HttpRequest> createHttpRequestWithEmptyBody(const std::string & url) {
    eos::common::HttpRequest::HeaderMap headers;
    size_t dataSize = 0;
    eos::common::HttpRequest::HeaderMap cookies;
    std::unique_ptr<eos::common::HttpRequest> request(new eos::common::HttpRequest(headers,"POST",url,"","",&dataSize,cookies));
    return request;
  }

  static std::unique_ptr<TapeRestApiConfig> createConfig(const std::string & accessURL, const std::string & siteName) {
    std::unique_ptr<TapeRestApiConfig> ret = std::make_unique<TapeRestApiConfig>(accessURL);
    ret->setSiteName(siteName);
    return ret;
  }

  static std::unique_ptr<TapeRestApiConfig> createConfig(const std::string & accessURL) {
    std::unique_ptr<TapeRestApiConfig> ret = std::make_unique<TapeRestApiConfig>(accessURL);
    return ret;
  }

  static const std::unique_ptr<TapeRestApiConfig> createDefaultConfig() {
    return createConfig("/api/","sitename");
  }
};

#endif // EOS_RESTAPITEST_HH
