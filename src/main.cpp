
#include <assert.h>
#include <string>
#include "url.h"

int main() {
  URL test("http://example.org/index.html");
  test.request();
  return 0;
}
