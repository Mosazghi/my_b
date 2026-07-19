#include "ResourceLoader.hpp"
#include <memory>
#include "file/File.hpp"
using namespace my_b;

namespace my_b::loader {

ResourceLoader::ResourceLoader(std::shared_ptr<http::IHttpClient> client)
    : m_http_client(std::move(client)) {}
ResourceLoader::~ResourceLoader() = default;

// TODO: Refactor to include generics instead
http::HttpResult ResourceLoader::load(const url::URL& url) {
  http::HttpResult resp{};

  const auto apply_about_blank_details = [](http::HttpResult& r) {
    r.response.status_line = http::HttpStatusLine{
        .version = "HTTP/1.1", .explanation = "OK", .status = 200};
    r.response.body = "<html><body><h1>about:blank</h1></body></html>";
    r.response.headers["content-type"] = "text/html";
  };

  using url::Scheme;
  switch (url.scheme) {
    case Scheme::HTTP:
    case Scheme::HTTPS:
    case Scheme::VIEW_SOURCE:
      resp = m_http_client->get(url.url);
      break;
    case Scheme::FILE: {
      auto file = file::read(url.url);
      if (file.has_value()) {
        resp.response.status_line = http::HttpStatusLine{
            .version = "HTTP/1.1", .explanation = "OK", .status = 200};
        resp.response.body = file.value();
      } else {
        resp.errors.emplace_back("File not found");
        resp.response.status_line = http::HttpStatusLine{
            .version = "HTTP/1.1", .explanation = "Not Found", .status = 404};
      }
      break;
    }
    case Scheme::DATA:
    // TODO: Add support for data scheme
    case Scheme::UNKNOWN:
    default:
      resp.errors.emplace_back("Unsupported URL scheme");
      apply_about_blank_details(resp);
      break;
  }
  return resp;
}
}  // namespace my_b::loader
