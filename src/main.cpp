#include <assert.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include "http/HttpClient.h"
#include "url/Url.h"

static Logger* logger = new Logger("main");

void print_usage();

int main(int argc, char* argv[]) {
  std::string url_addr{};

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " --url <url>\n";
    exit(EXIT_FAILURE);
  }

  for (int i = 1; i < argc; i++) {
    const std::string& arg{argv[i]};

    if (arg == "--help" || arg == "-h") {
      print_usage();
      exit(EXIT_SUCCESS);
    }

    if (arg == "--url" || arg == "-u") {
      for (size_t j = i + 1; j < argc; ++j) {
        if (argv[j][0] == '-' || (argv[j][0] == '-' && argv[j][1] == '-')) {
          break;
        }
        url_addr.append(std::format("{} ", argv[j]));
      }
    }
  }

  if (url_addr.empty()) {
    logger->err("URL cannot be empty!");
    exit(EXIT_FAILURE);
  }

  auto http_client = std::make_shared<http::HttpClient>();
  auto file_client = std::make_shared<file::File>();
  url::URL url(url_addr, http_client, file_client);
  auto response = url.request();
  if (response) {
    url.show(response->body);
  }

  return 0;
}

void print_usage() {
  std::cout << "Usage: my_b -u|--url <url>\n";
  std::cout << "URL can be a HTTP address or data scheme (text/html) \n";
}
