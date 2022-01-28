#include "libmemintercept.h"
#include <cstdio>
#include <map>

struct AllocMap;
static AllocMap *alloc_map = nullptr;

struct AllocInfo {
  void *start;
  size_t len;
};

struct AllocMap : libmemintercept::MemCallback {
  struct AllocData {
    std::size_t len;
  };

  std::map<void *, AllocData> allocs;

  AllocMap() { alloc_map = this; }

  void memoryAllocated(void *addr, std::size_t size) override {
    AllocData data;
    data.len = size;
    allocs[addr] = data;
  }

  void memoryReallocated(void *old_addr, void *new_addr,
                         std::size_t new_size) override {
    memoryFreed(old_addr);
    memoryAllocated(new_addr, new_size);
  }

  void memoryFreed(void *addr) override { allocs.erase(addr); }

  AllocInfo getInfoForAddr(void *addr) {
    AllocInfo res;
    res.start = 0;
    res.len = 0;

    // FIXME: We could binary search here.
    for (const auto &p : allocs) {
      if (p.first > addr)
        return res;
      res.start = p.first;
      res.len = p.second.len;
    }
    return res;
  }
};

extern "C" {
void printAllocInfo(void *addr) {
  AllocInfo i = alloc_map->getInfoForAddr(addr);
  fprintf(stderr, "Info: %p -> %ld|\n", addr, i.len);
}
}

LIBMEMINTERCEPT_ADD_CB(AllocMap)
