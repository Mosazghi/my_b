#include "url.h"
#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <format>
#include <iostream>
#include "logging.h"

// example http://example.org/index.html
//          |        |           |
//       scheme     host       path
URL::URL(std::string const& url) {
  m_scheme = url.substr(0, url.find("://"));

  assert(m_scheme == "http" && "Scheme not http!");

  std::string const rest = url.substr(url.find("://") + 3);
  m_hostname = rest.substr(0, rest.find("/"));
  m_path = rest.substr(rest.find("/"));

  std::cout << "Scheme: " << m_scheme << '\n';
  std::cout << "Rest: " << rest << '\n';
  std::cout << "Host: " << m_hostname << '\n';
  std::cout << "Path: " << m_path << '\n';
  m_port = 80;
}

void URL::request() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    LOG_ERR("Socket creation failed");
    exit(EXIT_FAILURE);
  }
  addrinfo hints{};
  hints.ai_family = AF_INET;  // IPv4
  hints.ai_socktype = SOCK_STREAM;

  addrinfo* res;
  if (getaddrinfo(m_hostname.c_str(), std::to_string(m_port).c_str(), &hints,
                  &res) != 0) {
    LOG_ERR("DNS lookup failed");
    exit(EXIT_FAILURE);
  }

  // res->ai_addr is ready to use with connect()
  if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
    LOG_ERR("Connection failed");
    exit(EXIT_FAILURE);
  }

  std::string buf = std::format("GET {} HTTP/1.0\r\n", m_path);
  buf.append(std::format("Host: {}\r\n", m_hostname));
  buf.append("\r\n");

  send(sockfd, buf.c_str(), buf.size(), 0);

  char buffer[1024];
  read(sockfd, buffer, 1024);
  std::cout << "Message from server: " << buffer << std::endl;

  close(sockfd);
  freeaddrinfo(res);
}
