#pragma once

#include <gmock/gmock.h>
#include <format>
#include <string>
#include "http/IHttpClient.hpp"

inline std::string get_mock_data_file_path(std::string_view path) {
  return std::format("{}/mock_data/{}", TEST_DATA_DIR, path);
}

class MockHttpClient : public http::IHttpClient {
 public:
  MOCK_METHOD(http::HttpResult, get, (std::string_view url), (override));
};
