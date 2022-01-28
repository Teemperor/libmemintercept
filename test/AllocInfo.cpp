#include <cstdio>
#include <cstdlib>


extern "C" {
void printAllocInfo(void *addr);
}

int main() {
  void *x = malloc(1337);
  fprintf(stderr, "x =");
  printAllocInfo(x);
  fprintf(stderr, "x + 3 =");
  printAllocInfo((void*)(((int*)x) + 3));
  fprintf(stderr, "x - 1 =");
  printAllocInfo((void*)(((int*)x) - 1));
  free(x);
  fprintf(stderr, "x(free'd) =");
  printAllocInfo(x);
}
