#include <assert.h>
#include <iostream>
#include <memory>
#include <string>
#include "http_client.h"
#include "url.h"

static Logger* logger = new Logger("main");
int main(int argc, char* argv[]) {
  // logger->inf("Initializing...");
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <url>\n";
    return 1;
  }

  std::string addr = argv[1];
  auto http_client = std::make_shared<HttpClient>();
  URL url(addr, http_client);
  auto response = url.request();
  url.show(response.body);

  return 0;
}
