#include <assert.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include "http/HttpClient.h"
#include "url/Url.h"

static Logger* logger = new Logger("main");

static void print_usage();
static void parse_url(std::string& buffer_url, char** url, int start_idx,
                      int size);

int main(int argc, char* argv[]) {
  std::string url_addr{};
  bool print_output{};

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
      parse_url(url_addr, argv, i, argc);
    }

    if (arg == "--show" || arg == "-s") {
      print_output = true;
    }
  }

  if (url_addr.empty()) {
    logger->err("URL cannot be empty!");
    exit(EXIT_FAILURE);
  }

  auto http_client = std::make_shared<http::HttpClient>();
  url::URL url(url_addr, http_client);
  auto response = url.request();
  if (response && print_output) {
    url.show(response->body);
  }

  return 0;
}

static void parse_url(std::string& buffer_url, char** url, int start_idx,
                      int size) {
  for (size_t j = start_idx + 1; j < size; ++j) {
    if (url[j][0] == '-' || (url[j][0] == '-' && url[j][1] == '-')) {
      break;
    }
    // Here, we support the DATA scheme by checking if there are spaces in the
    // argument.
    if (j - start_idx > 1) {
      buffer_url.append(std::format("{} ", url[j]));

      // Scheme other than DATA
    } else {
      buffer_url.append(std::format("{}", url[j]));
    }
  }
}

static void print_usage() {
  std::cout << "Usage:\t my_b -u|--url <url>\n";
  std::cout << "OPTIONS:\n";
  std::cout << "\t-s|--show\t Print response output\n";
}
