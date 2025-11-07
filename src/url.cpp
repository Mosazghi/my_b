#include "url.h"
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <format>
#include <iostream>
#include "logger.h"
std::string http_req(Logger* logger, int m_port, const std::string& m_hostname,
                     const std::string& m_path);
std::string https_req(Logger* logger, int m_port, const std::string& m_hostname,
                      const std::string& m_path);

URL::URL(const std::string& url) {
  logger = new Logger("URL");
  auto sep1 = url.find("://");
  if (sep1 == std::string::npos) {
    logger->err("Invalid url! [://]");
    exit(EXIT_FAILURE);
  }
  m_scheme = url.substr(0, sep1);

  // assert(m_scheme == "http" && "Scheme not http!");

  std::string rest = url.substr(sep1 + 3);

  if (rest[rest.size() - 1] != '/') {
    rest.append("/");
  }

  auto sep2 = rest.find("/");
  logger->dbg(std::format("Rest: {}", rest));
  if (sep2 == std::string::npos) {
    logger->err("Invalid url! [/]");
    exit(EXIT_FAILURE);
  }
  // sep2 = "/";
  m_hostname = rest.substr(0, sep2);
  m_path = rest.substr(sep2);
  m_port = m_scheme == "http" ? 80 : 443;
  logger->dbg("Host: {}", m_hostname);
  logger->dbg("Scheme: {}", m_scheme);
  logger->dbg("Port: {}", m_port);
  logger->dbg("Path: {}", m_path);
}

std::string URL::request() {
  if (m_port == 80) {
    return http_req(logger, m_port, m_hostname, m_path);
  } else {
    return https_req(logger, m_port, m_hostname, m_path);
  }
}

std::string http_req(Logger* logger, int m_port, const std::string& m_hostname,
                     const std::string& m_path) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    logger->err("Connection failed!");
    exit(EXIT_FAILURE);
  }

  addrinfo hints{};
  hints.ai_family = AF_INET;  // IPv4
  hints.ai_socktype = SOCK_STREAM;

  addrinfo* res;
  if (getaddrinfo(m_hostname.c_str(), std::to_string(m_port).c_str(), &hints,
                  &res) != 0) {
    logger->err("DNS lookup failed");
    exit(EXIT_FAILURE);
  }

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
    logger->err("Connection failed");
    exit(EXIT_FAILURE);
  }

  std::string buf = std::format("GET {} HTTP/1.0\r\n", m_path);
  buf.append(std::format("Host: {}\r\n", m_hostname));
  buf.append("\r\n");
  send(sockfd, buf.c_str(), buf.size(), 0);

  char buffer[1024];
  read(sockfd, buffer, 1024);
  logger->dbg("Response: \n{}", buffer);
  // Parsing of response
  std::string response{buffer};
  // logger->inf("Response: \n{}", response);
  auto status_line = response.substr(0, response.find_first_of("\r\n"));
  // logger->inf("Status line: {}", status_line);

  close(sockfd);
  freeaddrinfo(res);
  return buffer;
}
std::string https_req(Logger* logger, int m_port, const std::string& m_hostname,
                      const std::string& m_path) {
  // OPENSSL --
  BIO* bio;
  SSL* ssl;
  SSL_CTX* ctx;

  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();

  ctx = SSL_CTX_new(SSLv23_client_method());

  if (ctx == NULL) {
    logger->err("SSL CTX is null!");
    exit(EXIT_FAILURE);
  }

  bio = BIO_new_ssl_connect(ctx);
  BIO_set_conn_hostname(
      bio, std::format("{}:{}", m_hostname, std::to_string(m_port)).c_str());

  if (BIO_do_connect(bio) <= 0) {
    logger->err("Failed connection");
    exit(EXIT_FAILURE);
  } else {
    logger->dbg("BIO connected");
  }

  std::string write_buf = std::format("GET {} HTTP/1.0\r\n", m_path);
  write_buf.append(std::format("Host: {}\r\n", m_hostname));
  write_buf.append("Connection: close\r\n");
  write_buf.append("\r\n");

  if (BIO_write(bio, write_buf.c_str(), strlen(write_buf.c_str())) <= 0) {
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
  char buf[1024];

  //
  //  Read the response message
  //
  for (;;) {
    //
    //  Get chunks of the response 1023 at the time.
    //
    size = BIO_read(bio, buf, 1023);

    //
    //  If no more data, then exit the loop
    //
    if (size <= 0) {
      break;
    }

    //
    //  Terminate the string with a 0, to let know C when the string
    //  ends.
    //
    buf[size] = 0;

    //
    //  ->  Print out the response
    //
    // printf("%s", buf);
  }
  BIO_free_all(bio);
  SSL_CTX_free(ctx);
  return buf;
}
