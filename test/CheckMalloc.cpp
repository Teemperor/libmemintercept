#include <cstdlib>
#include <cstdio>

int main() {
  void *x = malloc(1337);
  //printf("Allocated %p\n", x);
  free(x);
}
