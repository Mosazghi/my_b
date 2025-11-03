#include <assert.h>
#include <iostream>
#include <string>
#include "url.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <url>\n";
    return 1;
  }
  std::string url = argv[1];
  URL test(url);
  test.request();
  return 0;
}
