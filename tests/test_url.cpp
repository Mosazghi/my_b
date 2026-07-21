#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include "http/HttpClient.hpp"
#include "resource-loader/ResourceLoader.hpp"
#include "test_helpers.hpp"
#include "url/Url.hpp"

using namespace my_b;

class UrlTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_loader = std::make_unique<loader::ResourceLoader>(
        std::make_unique<http::HttpClient>());
    // this->loader = std::move(loader);
  }
  std::unique_ptr<loader::ResourceLoader> m_loader;

  void TearDown() override { m_loader = nullptr; }
};

TEST_F(UrlTest, HttpsValid) {
  auto url = url::URL("https://google.no/");
  auto resp = m_loader->load(url);

  EXPECT_TRUE(resp.is_success());
  EXPECT_NE(resp.response.body, "");
  EXPECT_EQ(resp.response.status_line.status, 200);
}

TEST_F(UrlTest, HttpsValidNoPathGiven) {
  auto url = url::URL("https://google.no");
  auto resp = m_loader->load(url);

  EXPECT_NE(resp.response.body, "");
  EXPECT_EQ(resp.response.status_line.status, 200);
}

TEST_F(UrlTest, HttpsInvalid) {
  auto url =
      url::URL("ttps://browser.engineering/examples/example1-simple.html/");
  auto resp = m_loader->load(url);

  EXPECT_FALSE(resp.is_success());
}

TEST(UrlParsing, DetectsHttpScheme) {
  auto url = url::URL("http://example.com/path");
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::HTTP));
}

TEST(UrlParsing, DetectsHttpsScheme) {
  auto url = url::URL("https://example.com/path");
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::HTTPS));
}

TEST(UrlParsing, DetectsFileSchemeAndStripsPrefix) {
  auto url = url::URL("file:///tmp/test.txt");
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::FILE));
  EXPECT_EQ(url.url, "/tmp/test.txt");
}

TEST(UrlParsing, DetectsDataSchemeAndSplitsProtocolFromPayload) {
  auto url = url::URL("data:text/html,<h1>hi</h1>");
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::DATA));
  EXPECT_EQ(url.data_scheme.protocol, "text/html");
  EXPECT_EQ(url.data_scheme.data, "<h1>hi</h1>");
}

TEST(UrlParsing, DetectsViewSourceSchemeAndStripsPrefix) {
  auto url = url::URL("view-source:https://example.com/");
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::VIEW_SOURCE));
  EXPECT_EQ(url.url, "https://example.com/");
}

TEST(UrlParsing, IsSchemeInVectorMatchesAnyGivenScheme) {
  auto url = url::URL("https://example.com/");
  EXPECT_TRUE(url.is_scheme_in({url::Scheme::HTTP, url::Scheme::HTTPS}));
  EXPECT_FALSE(url.is_scheme_in({url::Scheme::FILE, url::Scheme::DATA}));
}

TEST(UrlParsing, UnsupportedSchemeDefaultsToUnknown) {
  auto url = url::URL("ftp://example.com/file");
  EXPECT_TRUE(url.is_scheme_in(url::Scheme::UNKNOWN));
}

TEST(UrlParsing, MalformedUrlWithoutColonDefaultsToUnknown) {
  auto url = url::URL("not-a-url");
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

  auto url = url::URL("http://example.com/");
  auto loader =
      std::make_unique<loader::ResourceLoader>(std::move(mock_client));
  auto resp = loader->load(url);

  EXPECT_EQ(resp.response.body, "hello");
  EXPECT_EQ(resp.response.status_line.status, 200);
}

TEST_F(UrlTest, FileSchemeReadsExistingFile) {
  const auto path = get_mock_data_file_path("short-file.txt");
  auto url = url::URL("file://" + path);

  auto resp = m_loader->load(url);

  EXPECT_EQ(resp.response.status_line.status, 200);
  EXPECT_NE(resp.response.body.find("This is a short file."),
            std::string::npos);
}

TEST_F(UrlTest, FileSchemeMissingFileReturns404) {
  const auto path = get_mock_data_file_path("nonexistent-file.txt");
  auto url = url::URL("file://" + path);

  auto resp = m_loader->load(url);

  EXPECT_EQ(resp.response.status_line.status, 404);
  EXPECT_TRUE(resp.has_error());
}

TEST_F(UrlTest, UnsupportedSchemeReturnsAboutBlankFallback) {
  auto url = url::URL("ftp://example.com/file");

  auto resp = m_loader->load(url);

  EXPECT_TRUE(resp.has_error());
  EXPECT_NE(resp.response.body.find("about:blank"), std::string::npos);
}
