#include <cassert>
#include <cstdio>
#include <dlfcn.h>
#include <cstdlib>
#include <mutex>

#include "libmemintercept.h"

namespace {

static unsigned settingUpInternalState = 0;

struct InitGuard {
  InitGuard() { settingUpInternalState++; }
  ~InitGuard() { settingUpInternalState--; }
};

static bool isSettingUpInternalState() {
  return settingUpInternalState;
}

struct StartMemCallback : public libmemintercept::MemCallback {
};
static StartMemCallback startCallback;
} // namespace

namespace libmemintercept {
void createAndInstallMemCallback(MemCallback *(*createCallback)()) {
  InitGuard guard;
  MemCallback *c = createCallback();
  startCallback.chainUpNext(c);
}
}

static void *(*real_malloc)(size_t) = nullptr;
static void (*real_free)(void *) = nullptr;

template<typename T>
static void loadRealImplIfNeeded(T &t, const char *name) {
  if (t)
    return;

  t = reinterpret_cast<T>(dlsym(RTLD_NEXT, name));
  if (!t) {
    // FIXME: Put an error message here.
    abort();
  }
}

#define LOAD_REAL_IMPL(NAME) loadRealImplIfNeeded(real_##NAME, #NAME)

static std::recursive_mutex internalStateMutex;

extern "C" {
void *malloc(size_t size) {
  std::lock_guard guard(internalStateMutex);

  LOAD_REAL_IMPL(malloc);
  void *x = real_malloc(size);
  if (!isSettingUpInternalState()) {
    InitGuard initGuard;
    libmemintercept::MemCallback *cb = &startCallback;
    do {
      cb->memoryAllocated(x, size);
      cb = cb->getNext();
    } while (cb);
  }
  return x;
}

void free(void *p) {
  if (p == nullptr)
    return;

  std::lock_guard guard(internalStateMutex);
  LOAD_REAL_IMPL(free);
  if (!isSettingUpInternalState()) {
    InitGuard initGuard;
    libmemintercept::MemCallback *cb = &startCallback;
    do {
      cb->memoryFreed(p);
      cb = cb->getNext();
    } while (cb);
  }
  real_free(p);
}
#undef LOAD_REAL_IMPL
}
