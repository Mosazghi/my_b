
#include <gtest/gtest.h>
#include "http/HttpClient.h"
#include "http/Types.h"

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
  auto resp = m_http_client->get("http://httpforever.com/");

  EXPECT_TRUE(resp.has_value());
  EXPECT_NE(resp->body, "");
  EXPECT_EQ(resp->code, 200);
}

TEST_F(HttpTest, GetReq404Failure) {
  auto resp = m_http_client->get("http://httpforeverr.com/");

  EXPECT_FALSE(resp.has_value());
}

TEST_F(HttpTest, GetReqSuccessHttps) {
  auto resp = m_http_client->get("https://portfolio.mostes.no/");

  EXPECT_TRUE(resp.has_value());
  EXPECT_NE(resp->body, "");
  EXPECT_EQ(resp->code, 200);
}
