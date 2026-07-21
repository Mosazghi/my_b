#pragma once

#include <gmock/gmock.h>
#include <format>
#include <string>
#include "http/HttpClient.hpp"

inline std::string get_mock_data_file_path(std::string_view path) {
  return std::format("{}/mock_data/{}", TEST_DATA_DIR, path);
}

class MockHttpClient : public my_b::http::IHttpClient {
 public:
  MOCK_METHOD(my_b::http::HttpResult, get, (std::string_view url), (override));
};
