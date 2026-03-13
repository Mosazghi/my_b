#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "http/HttpClient.hpp"
#include "http/Types.hpp"

class HttpTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    m_http_client = std::make_unique<http::HttpClient>();
  }
  std::unique_ptr<http::HttpClient> m_http_client;
};

TEST_F(HttpTest, GetReqSuccess) {
  auto resp = m_http_client->get("http://httpforever.com/");

  EXPECT_NE(resp.response.body, "");
  EXPECT_EQ(resp.response.status_line.status, 200);
}

TEST_F(HttpTest, GetReq404Failure) {
  auto resp = m_http_client->get("http://httpforeverr.com/");

  EXPECT_FALSE(resp.response.status_line.status == 200);
  EXPECT_TRUE(resp.has_error());
}

TEST_F(HttpTest, GetReqSuccessHttps) {
  auto resp = m_http_client->get("https://portfolio.mostes.no/");

  EXPECT_FALSE(resp.has_error());
  EXPECT_NE(resp.response.body, "");
  EXPECT_EQ(resp.response.status_line.status, 200);
}

// -- Test redirect handling --
TEST_F(HttpTest, GetReqRedirect) {
  auto resp = m_http_client->get("https://browser.engineering/redirect");
  EXPECT_FALSE(resp.has_error());
  EXPECT_EQ(resp.redirect_count, 1);

  auto has_moved_permanently = resp.response.body.find(
      "<head><title>301 Moved Permanently</title></head>");
  EXPECT_EQ(has_moved_permanently, std::string::npos);
  EXPECT_EQ(resp.response.status_line.status, 200);
}
TEST_F(HttpTest, GetReqRedirect2) {
  auto resp = m_http_client->get("https://browser.engineering/redirect2");
  EXPECT_FALSE(resp.has_error());
  EXPECT_EQ(resp.redirect_count, 2);

  auto has_moved_permanently = resp.response.body.find(
      "<head><title>301 Moved Permanently</title></head>");
  EXPECT_EQ(has_moved_permanently, std::string::npos);
  EXPECT_EQ(resp.response.status_line.status, 200);
}

TEST_F(HttpTest, GetReqRedirect3) {
  auto resp = m_http_client->get("https://browser.engineering/redirect3");
  EXPECT_FALSE(resp.has_error());
  EXPECT_EQ(resp.redirect_count, 3);

  auto has_moved_permanently = resp.response.body.find(
      "<head><title>301 Moved Permanently</title></head>");
  EXPECT_EQ(has_moved_permanently, std::string::npos);
  EXPECT_EQ(resp.response.status_line.status, 200);
}
