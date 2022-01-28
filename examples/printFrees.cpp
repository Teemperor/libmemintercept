#include "libmemintercept.h"
#include <cstdio>

struct PrintFrees : libmemintercept::MemCallback {
  void memoryFreed(void *addr) override { fprintf(stderr, "free(%p)\n", addr); }
};

LIBMEMINTERCEPT_ADD_CB(PrintFrees)

struct PrintMallocs : libmemintercept::MemCallback {
  void memoryAllocated(void *addr, std::size_t size) override {
    fprintf(stderr, "malloc(%ld) = %p\n", size, addr);
  }
};

LIBMEMINTERCEPT_ADD_CB(PrintMallocs)

struct PrintReallocs : libmemintercept::MemCallback {
  void memoryReallocated(void *old, void *new_addr, std::size_t size) override {
    fprintf(stderr, "realloc(%p, %ld) = %p\n", old, size, new_addr);
  }
};

LIBMEMINTERCEPT_ADD_CB(PrintReallocs)
