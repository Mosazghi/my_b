#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "http/HttpClient.hpp"
#include "test_helpers.hpp"
#include "url/Url.hpp"

class UrlTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_http_client = std::make_shared<http::HttpClient>();
  }
  std::shared_ptr<http::HttpClient> m_http_client;

  void TearDown() override { m_http_client = nullptr; }
};

TEST_F(UrlTest, HttpsValid) {
  auto url = url::URL("https://google.no/", m_http_client);
  auto resp = url.request();

  // EXPECT_TRUE(resp.has_value());
  EXPECT_NE(resp.response.body, "");
  EXPECT_EQ(resp.response.status_line.status, 200);
}

TEST_F(UrlTest, HttpsValidNoPathGiven) {
  auto url = url::URL("https://google.no", m_http_client);
  auto resp = url.request();

  EXPECT_NE(resp.response.body, "");
  EXPECT_EQ(resp.response.status_line.status, 200);
}

TEST_F(UrlTest, HttpsInvalid) {
  auto url =
      url::URL("ttps://browser.engineering/examples/example1-simple.html/",
               m_http_client);
  auto resp = url.request();

  EXPECT_FALSE(resp.is_success());
}

// -- Scheme parsing (offline, no client needed) --

TEST(UrlParsing, DetectsHttpScheme) {
  auto url = url::URL("http://example.com/path", nullptr);
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::HTTP));
}

TEST(UrlParsing, DetectsHttpsScheme) {
  auto url = url::URL("https://example.com/path", nullptr);
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::HTTPS));
}

TEST(UrlParsing, DetectsFileSchemeAndStripsPrefix) {
  auto url = url::URL("file:///tmp/test.txt", nullptr);
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::FILE));
  EXPECT_EQ(url.m_url, "/tmp/test.txt");
}

TEST(UrlParsing, DetectsDataSchemeAndSplitsProtocolFromPayload) {
  auto url = url::URL("data:text/html,<h1>hi</h1>", nullptr);
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::DATA));
  EXPECT_EQ(url.m_data.data_scheme.protocol, "text/html");
  EXPECT_EQ(url.m_data.data_scheme.data, "<h1>hi</h1>");
}

TEST(UrlParsing, DetectsViewSourceSchemeAndStripsPrefix) {
  auto url = url::URL("view-source:https://example.com/", nullptr);
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::VIEW_SOURCE));
  EXPECT_EQ(url.m_url, "https://example.com/");
}

TEST(UrlParsing, IsSchemeInVectorMatchesAnyGivenScheme) {
  auto url = url::URL("https://example.com/", nullptr);
  EXPECT_TRUE(url.is_scheme_in({url::Scheme::HTTP, url::Scheme::HTTPS}));
  EXPECT_FALSE(url.is_scheme_in({url::Scheme::FILE, url::Scheme::DATA}));
}

TEST(UrlParsing, UnsupportedSchemeDefaultsToUnknown) {
  auto url = url::URL("ftp://example.com/file", nullptr);
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::UNKNOWN));
}

TEST(UrlParsing, MalformedUrlWithoutColonDefaultsToUnknown) {
  auto url = url::URL("not-a-url", nullptr);
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::UNKNOWN));
}

// -- request() dispatch (offline, mocked or local file client) --

TEST(UrlRequestDispatch, HttpSchemeDelegatesToHttpClient) {
  auto mock_client = std::make_shared<MockHttpClient>();

  http::HttpResult expected{};
  expected.response.status_line = {
      .version = "HTTP/1.1", .explanation = "OK", .status = 200};
  expected.response.body = "hello";

  EXPECT_CALL(*mock_client, get(std::string_view("http://example.com/")))
      .WillOnce(::testing::Return(expected));

  auto url = url::URL("http://example.com/", mock_client);
  auto resp = url.request();

  EXPECT_EQ(resp.response.body, "hello");
  EXPECT_EQ(resp.response.status_line.status, 200);
}

TEST(UrlRequestDispatch, FileSchemeReadsExistingFile) {
  const auto path = get_mock_data_file_path("short-file.txt");
  auto url = url::URL("file://" + path, nullptr);

  auto resp = url.request();

  EXPECT_EQ(resp.response.status_line.status, 200);
  EXPECT_NE(resp.response.body.find("This is a short file."),
            std::string::npos);
}

TEST(UrlRequestDispatch, FileSchemeMissingFileReturns404) {
  auto url = url::URL("file:///nonexistent/path/file.txt", nullptr);

  auto resp = url.request();

  EXPECT_EQ(resp.response.status_line.status, 404);
  EXPECT_TRUE(resp.has_error());
}

TEST(UrlRequestDispatch, DataSchemeReturnsAboutBlankFallback) {
  auto url = url::URL("data:text/plain,hello", nullptr);

  auto resp = url.request();

  EXPECT_TRUE(resp.has_error());
  EXPECT_NE(resp.response.body.find("about:blank"), std::string::npos);
}

TEST(UrlRequestDispatch, UnsupportedSchemeReturnsAboutBlankFallback) {
  auto url = url::URL("ftp://example.com/file", nullptr);

  auto resp = url.request();

  EXPECT_TRUE(resp.has_error());
  EXPECT_NE(resp.response.body.find("about:blank"), std::string::npos);
}
