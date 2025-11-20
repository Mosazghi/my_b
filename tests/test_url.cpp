

#include <gtest/gtest.h>
#include "file.h"
#include "http_client.h"
#include "url.h"

class UrlTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    m_http_client = std::make_shared<http::HttpClient>();
    m_file_client = std::make_shared<file::File>();
  }
  std::shared_ptr<http::HttpClient> m_http_client;
  std::shared_ptr<file::File> m_file_client;

  virtual void TearDown() override {
    m_http_client = nullptr;
    m_file_client = nullptr;
  }
};

TEST_F(UrlTest, HttpsValid) {
  auto url = new url::URL("https://portfolio.mostes.no/", m_http_client,
                          m_file_client);
  auto resp = url->request();

  EXPECT_TRUE(resp.has_value());
  EXPECT_NE(resp->body, "");
  EXPECT_EQ(resp->code, 200);
}

TEST_F(UrlTest, HttpsValidNoPathGiven) {
  auto url =
      new url::URL("https://portfolio.mostes.no", m_http_client, m_file_client);
  auto resp = url->request();

  EXPECT_TRUE(resp.has_value());
  EXPECT_NE(resp->body, "");
  EXPECT_EQ(resp->code, 200);
}

TEST_F(UrlTest, HttpsInvalid) {
  auto url =
      new url::URL("ttps://browser.engineering/examples/example1-simple.html/",
                   m_http_client, m_file_client);
  auto resp = url->request();

  EXPECT_FALSE(resp.has_value());
}
