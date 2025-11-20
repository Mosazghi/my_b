#include <assert.h>
#include <iostream>
#include <memory>
#include <string>
#include "http_client.h"
#include "url.h"

static Logger* logger = new Logger("main");
int main(int argc, char* argv[]) {
  std::cout << "Init.";
  logger->inf("Initializing...");
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <url>\n";
    return 1;
  }
  for (int i = 1; i < argc; i++) {
    std::cout << argv[i] << '\n';
  }

  std::string addr = argv[1];
  auto http_client = std::make_shared<http::HttpClient>();
  auto file_client = std::make_shared<file::File>();
  url::URL url(addr, http_client, file_client);
  auto response = url.request();
  if (response) {
    // logger->inf("Body {}", response->body);
    url.show(response->body);
  }

  return 0;
}
