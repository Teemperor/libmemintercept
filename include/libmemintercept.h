#ifndef LIBMEMINTERCEPT_H
#define LIBMEMINTERCEPT_H

#include <cstdint>

namespace libmemintercept {
class MemCallback {
  MemCallback *next = nullptr;
public:
  void chainUpNext(MemCallback *n) {
    if (next != nullptr) {
      next->chainUpNext(n);
      return;
    }
    next = n;
  }
  MemCallback *getNext() {
    return next;
  }
  virtual void memoryAllocated(void *addr, std::size_t size) {
    (void)addr;
    (void)size;
  }
  virtual void memoryFreed(void *addr) {
    (void)addr;
  }
};

void createAndInstallMemCallback(MemCallback *(*createCallback)());
}

#define LIBMEMINTERCEPT_ADD_CB(CBNAME) \
  namespace { \
  static libmemintercept::MemCallback *createCB##CBNAME() { \
    return new CBNAME(); \
  } \
  struct StartupCB##CBNAME { \
    StartupCB##CBNAME() { \
      libmemintercept::createAndInstallMemCallback(&createCB##CBNAME); \
    } \
  }; \
  } \
  StartupCB##CBNAME libmemintercept_startup##CBNAME;


#endif // LIBMEMINTERCEPT_H
