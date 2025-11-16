
#include <gtest/gtest.h>
#include "http_client.h"

class HttpTest : public ::testing::Test {
 protected:
  virtual void SetUp() override { m_http_client = new http::HttpClient(); }
  http::HttpClient* m_http_client = nullptr;

  virtual void TearDown() override {
    delete m_http_client;
    m_http_client = nullptr;
  }
};

TEST_F(HttpTest, GetReqSuccess) {
  auto response = m_http_client->get({
      .port = 80,
      .hostname = "httpforever.com",
      .path = "/",
      .scheme = url::Scheme::HTTP,
  });

  EXPECT_TRUE(response.has_value());
  EXPECT_NE(response->body, "");
  EXPECT_EQ(response->code, 200);
}

TEST_F(HttpTest, GetReq404Failure) {
  auto response = m_http_client->get({
      .port = 80,
      .hostname = "httpforeverr.com",
      .path = "/",
      .scheme = url::Scheme::HTTP,
  });

  EXPECT_FALSE(response.has_value());
}

TEST_F(HttpTest, GetReqSuccessHttps) {
  auto response = m_http_client->get({
      .port = 443,
      .hostname = "portfolio.mostes.no",
      .path = "/",
      .scheme = url::Scheme::HTTPS,
  });

  EXPECT_TRUE(response.has_value());
  EXPECT_NE(response->body, "");
  EXPECT_EQ(response->code, 200);
}
