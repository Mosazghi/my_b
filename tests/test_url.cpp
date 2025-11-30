#include <gtest/gtest.h>
#include "http/HttpClient.h"
#include "url/Url.h"

class UrlTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    m_http_client = std::make_shared<http::HttpClient>();
  }
  std::shared_ptr<http::HttpClient> m_http_client;

  virtual void TearDown() override { m_http_client = nullptr; }
};

TEST_F(UrlTest, HttpsValid) {
  auto url = new url::URL("https://portfolio.mostes.no/", m_http_client);
  auto resp = url->request();

  EXPECT_TRUE(resp.has_value());
  EXPECT_NE(resp->body, "");
  EXPECT_EQ(resp->code, 200);
}

TEST_F(UrlTest, HttpsValidNoPathGiven) {
  auto url = new url::URL("https://portfolio.mostes.no", m_http_client);
  auto resp = url->request();

  EXPECT_TRUE(resp.has_value());
  EXPECT_NE(resp->body, "");
  EXPECT_EQ(resp->code, 200);
}

TEST_F(UrlTest, HttpsInvalid) {
  auto url =
      new url::URL("ttps://browser.engineering/examples/example1-simple.html/",
                   m_http_client);
  auto resp = url->request();

  EXPECT_FALSE(resp.has_value());
}
