#include <assert.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include "http_client.h"
#include "url.h"

static Logger* logger = new Logger("main");
void print_help(char* argv[]);

int main(int argc, char* argv[]) {
  std::string url_addr{};
  logger->inf("Initializing...");

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " --url <url>\n";
    exit(EXIT_FAILURE);
  }

  for (int i = 1; i < argc; i++) {
    const std::string& arg{argv[i]};
    if (arg == "--help" || arg == "-h") {
      print_help(argv);
      exit(EXIT_SUCCESS);
    }

    if (arg == "--url" || arg == "-u") {
      url_addr = argv[i + 1];
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

void print_help(char* argv[]) {
  std::cout << "Usage: " << argv[0] << " -u|--url <url>\n";
}
