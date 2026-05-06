#include <SFML/Graphics.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include "browser/Browser.hpp"
#include "common.hpp"
#include "http/HttpClient.hpp"
#include "url/Url.hpp"

static Logger& logger = Logger::getInstance();

static void print_usage();
static void parse_url(std::string& buffer_url, char** url, int start_idx,
                      int size);

int main(int argc, char** argv) {
  std::string url_addr{};

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " --url <url>\n";
    exit(EXIT_FAILURE);
  }

  for (int i = 1; i < argc; i++) {
    std::string_view arg{argv[i]};

    if (arg == "--help" || arg == "-h") {
      print_usage();
      exit(EXIT_SUCCESS);
    }

    if (arg == "--url" || arg == "-u") {
      parse_url(url_addr, argv, i, argc);
    }
  }

  if (url_addr.empty()) {
    logger.err("URL cannot be empty!");
    exit(EXIT_FAILURE);
  }
  sf::RenderWindow window(sf::VideoMode(1280, 720), "My Browser",
                          sf::Style::Default | sf::Style::Resize);
  window.setFramerateLimit(60);

  auto browser = std::make_unique<browser::Browser>(window);
  auto url = url::URL(url_addr, std::make_shared<http::HttpClient>());
  browser->load(url);
  browser->spin();
  return 0;
}

static void parse_url(std::string& buffer_url, char** url, int start_idx,
                      int size) {
  for (int j = start_idx + 1; j < size; ++j) {
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
