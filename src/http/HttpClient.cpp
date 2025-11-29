#include "HttpClient.h"
#include <arpa/inet.h>
#include <assert.h>
#include <http/HttpClient.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <format>
#include <optional>
#include <unordered_map>
#include "Types.h"
#include "logger.h"
#include "url/Url.h"
#include "utils.h"

namespace http {

HttpClient::HttpClient() { logger = new Logger("HttpClient"); }

HttpClient::~HttpClient() {}

std::optional<HttpResponse> HttpClient::get(const std::string& url) const {
  std::optional<http::HttpResponse> resp{};
  auto params = get_params_from_url(url);

  if (!params.has_value()) {
    return {};
  }

  std::string buffer = std::format("GET {} HTTP/1.0\r\n", params.value().path);
  buffer.append(std::format("Host: {}\r\n", params.value().hostname));
  buffer.append("User-Agent: mosa\r\n");
  buffer.append("Connection: close\r\n");
  buffer.append("\r\n");

  logger->dbg("Sending:\n{}", buffer);

  switch (params.value().scheme) {
    case url::Scheme::HTTP:
      resp = http_req(params.value(), buffer);
      break;
    case url::Scheme::HTTPS:
      resp = https_req(params.value(), buffer);
      break;
    default:
      logger->err("Unknown scheme");
      break;
  }

  return resp;
}

HttpResponse HttpClient::get_response_from_body(
    const std::string& response) const {
  if (response.empty()) {
    return {.code = 404, .body = response};
  }

  HttpStatusLine status_line{};
  std::unordered_map<std::string, std::string> headers{};

  std::string line{};
  std::istringstream iss(response);
  int i{};
  bool body_found = false;
  std::string body;
  while (std::getline(iss, line)) {
    if (i == 0) {
      std::vector<std::string> stats_split = utils::split_string(line, ' ');
      status_line.version = stats_split.at(0);

      try {
        status_line.status = std::stoi(stats_split.at(1));
      } catch (const std::invalid_argument& e) {
        logger->err("Error occured during parsing of code: {}", e.what());
        status_line.status = 400;
      }
      status_line.explaination = stats_split.at(2);

      logger->dbg("{}, {} {}", status_line.version, status_line.status,
                  status_line.explaination);
    } else {
      auto split_idx = line.find_first_of(":");
      if (split_idx != std::string::npos && !body_found) {
        std::string key = line.substr(0, split_idx);

        assert(key != "transfer-encoding" ||
               key != "content-encoding" &&
                   "These headers are not yet supported!");

        std::string val = line.substr(split_idx + 1);
        headers[key] = val;
        logger->dbg("{} - {}", key, val);
      } else {
        utils::trim(line);
        body.append(line + "\n");
        body_found = true;
      }
    }
    i++;
  }
  return {.code = status_line.status, .body = body};
}

std::optional<http::HttpReqParams> HttpClient::get_params_from_url(
    const std::string& url) const {
  http::HttpReqParams params{};
  auto s1 = url.find("://");
  std::string scheme = url.substr(0, s1);

  std::string rest = url.substr(s1 + 3);

  auto s2 = rest.find("/");

  if (scheme == "http") {
    params.scheme = url::Scheme::HTTP;
  } else if (scheme == "https") {
    params.scheme = url::Scheme::HTTPS;
  } else {
    logger->warn("Unknwon scheme");
    return {};
  }

  if (s2 == std::string::npos) {
    params.path = "/";
    params.hostname = url.substr(s1 + 3);
  } else {
    params.hostname = rest.substr(0, s2);
    params.path = rest.substr(s2);
  }

  // Custom port
  auto c_port = params.hostname.find_first_of(":");
  if (c_port != std::string::npos) {
    params.port = std::stoi(params.hostname.substr(c_port + 1));
    params.hostname = params.hostname.substr(0, c_port);
  } else {
    params.port = params.scheme == url::Scheme::HTTP ? 80 : 443;
  }

  logger->dbg("Host: {}", params.hostname);
  logger->dbg("Port: {}", params.port);
  logger->dbg("Path: {}", params.path);
  logger->dbg("Scheme: {}",
              params.scheme == url::Scheme::HTTP ? "HTTP" : "HTTPS");
  return params;
}

std::optional<HttpResponse> HttpClient::http_req(
    HttpReqParams params, const std::string& buffer) const {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    logger->err("Connection failed!");
    exit(EXIT_FAILURE);
  }

  addrinfo hints{};
  hints.ai_family = AF_INET;  // IPv4
  hints.ai_socktype = SOCK_STREAM;

  addrinfo* res;
  if (getaddrinfo(params.hostname.c_str(), std::to_string(params.port).c_str(),
                  &hints, &res) != 0) {
    logger->warn("DNS lookup failed");
    return std::nullopt;
  }

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
    logger->warn("Connection failed");
    return std::nullopt;
  }

  send(sockfd, buffer.c_str(), buffer.size(), 0);

  char recv_buf[1024];
  int size = read(sockfd, recv_buf, 1024);
  if (size == -1) {
    logger->warn("Encountered Err");
    return std::nullopt;
  }

  std::string response{recv_buf};
  auto parsed = get_response_from_body(response);

  close(sockfd);
  freeaddrinfo(res);
  return parsed;
}

// https://stackoverflow.com/questions/1011339/how-do-you-make-a-http-request-with-c
std::optional<HttpResponse> HttpClient::https_req(
    HttpReqParams params, const std::string& buffer) const {
  // OPENSSL --
  BIO* bio;
  SSL_CTX* ctx;

  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();

  ctx = SSL_CTX_new(SSLv23_client_method());

  if (ctx == NULL) {
    logger->warn("SSL CTX is null!");
    return std::nullopt;
  }

  bio = BIO_new_ssl_connect(ctx);
  BIO_set_conn_hostname(
      bio, std::format("{}:{}", params.hostname, std::to_string(params.port))
               .c_str());

  if (BIO_do_connect(bio) <= 0) {
    logger->warn("Failed connection");
    return std::nullopt;
  }

  if (BIO_write(bio, buffer.c_str(), strlen(buffer.c_str())) <= 0) {
    //
    //  Handle failed writes here
    //
    if (!BIO_should_retry(bio)) {
      // Not worth implementing, but worth knowing.
    }

    //
    //  -> Let us know about the failed writes
    //
    printf("Failed write\n");
  }

  //
  //  Variables used to read the response from the server
  //
  int size;

  char buf_[1024];
  std::string response;

  //
  //  Read the response message
  //
  for (;;) {
    //
    //  Get chunks of the response 1023 at the time.
    //
    size = BIO_read(bio, buf_, 1023);

    //
    //  If no more data, then return std::nullopt;
    //
    if (size <= 0) {
      break;
    }

    //
    //  Terminate the string with a 0, to let know C when the string
    //  ends.
    //
    buf_[size] = 0;

    //
    //  ->  Print out the response
    //
    response.append(buf_);
  }
  BIO_free_all(bio);

  SSL_CTX_free(ctx);

  auto parsed = get_response_from_body(response);
  return parsed;
}

}  // namespace http
