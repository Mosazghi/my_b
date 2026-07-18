#include <gtest/gtest.h>
#include "http/HttpRequest.hpp"

TEST(HttpMethodToString, MapsEachKnownMethod) {
  EXPECT_EQ(http::http_method_to_string(http::HttpMethod::GET), "GET");
  EXPECT_EQ(http::http_method_to_string(http::HttpMethod::POST), "POST");
  EXPECT_EQ(http::http_method_to_string(http::HttpMethod::PUT), "PUT");
  EXPECT_EQ(http::http_method_to_string(http::HttpMethod::DELETE), "DELETE");
}

TEST(HttpRequestBuilder, BuildsRequestLineWithMethodAndPath) {
  auto request = http::HttpRequestBuilder{}
                     .with_method(http::HttpMethod::GET)
                     .with_path("/index.html")
                     .build();

  EXPECT_EQ(request, "GET /index.html HTTP/1.1\r\n\r\n");
}

TEST(HttpRequestBuilder, IncludesSingleHeader) {
  auto request = http::HttpRequestBuilder{}
                     .with_method(http::HttpMethod::GET)
                     .with_path("/")
                     .with_header("Host", "example.com")
                     .build();

  EXPECT_EQ(request, "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
}

TEST(HttpRequestBuilder, IncludesBodyAfterBlankLine) {
  auto request = http::HttpRequestBuilder{}
                     .with_method(http::HttpMethod::POST)
                     .with_path("/submit")
                     .with_header("Content-Length", "4")
                     .with_body("data")
                     .build();

  EXPECT_EQ(request, "POST /submit HTTP/1.1\r\nContent-Length: 4\r\n\r\ndata");
}

TEST(HttpRequestBuilder, WithHeaderOverwritesExistingKey) {
  auto request = http::HttpRequestBuilder{}
                     .with_method(http::HttpMethod::GET)
                     .with_path("/")
                     .with_header("Host", "first.com")
                     .with_header("Host", "second.com")
                     .build();

  EXPECT_EQ(request, "GET / HTTP/1.1\r\nHost: second.com\r\n\r\n");
}

TEST(HttpResult, SuccessOn2xxWithNoErrors) {
  http::HttpResult result{};
  result.response.status_line.status = 200;

  EXPECT_TRUE(result.is_success());
  EXPECT_FALSE(result.has_error());
}

TEST(HttpResult, NotSuccessOn3xxButNotAnErrorEither) {
  http::HttpResult result{};
  result.response.status_line.status = 301;

  EXPECT_FALSE(result.is_success());
  EXPECT_FALSE(result.has_error());
}

TEST(HttpResult, ErrorOn4xxAndAbove) {
  http::HttpResult result{};
  result.response.status_line.status = 404;

  EXPECT_TRUE(result.has_error());
  EXPECT_FALSE(result.is_success());
}

TEST(HttpResult, ErrorWhenErrorsListNonEmptyRegardlessOfStatus) {
  http::HttpResult result{};
  result.response.status_line.status = 200;
  result.errors.emplace_back("Connection failed");

  EXPECT_TRUE(result.has_error());
  EXPECT_FALSE(result.is_success());
}

TEST(HttpResult, StatusBoundaryAt200And300) {
  http::HttpResult below{};
  below.response.status_line.status = 199;
  EXPECT_FALSE(below.is_success());

  http::HttpResult lower_bound{};
  lower_bound.response.status_line.status = 200;
  EXPECT_TRUE(lower_bound.is_success());

  http::HttpResult upper_bound{};
  upper_bound.response.status_line.status = 299;
  EXPECT_TRUE(upper_bound.is_success());

  http::HttpResult above{};
  above.response.status_line.status = 300;
  EXPECT_FALSE(above.is_success());
}
