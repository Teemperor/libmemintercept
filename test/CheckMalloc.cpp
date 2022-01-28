#include <cstdio>
#include <cstdlib>

int main() {
  void *x = malloc(1337);
  x = realloc(x, 1377);
  void *f = calloc(7331, 1);
  free(x);
}
