#include <cstdlib>

extern "C" {
void printAllocInfo(void *addr) {
  abort();
}
}
