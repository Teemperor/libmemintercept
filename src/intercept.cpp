#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <mutex>

#include "libmemintercept.h"

namespace {

static unsigned settingUpInternalState = 0;

struct InitGuard {
  InitGuard() { settingUpInternalState++; }
  ~InitGuard() { settingUpInternalState--; }
};

static bool isSettingUpInternalState() { return settingUpInternalState; }

struct StartMemCallback : public libmemintercept::MemCallback {};
static StartMemCallback startCallback;
} // namespace

namespace libmemintercept {
void createAndInstallMemCallback(MemCallback *(*createCallback)()) {
  InitGuard guard;
  MemCallback *c = createCallback();
  startCallback.chainUpNext(c);
}
} // namespace libmemintercept

static void *(*real_malloc)(size_t) = nullptr;
static void *(*real_calloc)(size_t, size_t) = nullptr;
static void *(*real_realloc)(void *, size_t) = nullptr;
static void (*real_free)(void *) = nullptr;

template <typename T> static void loadRealImplIfNeeded(T &t, const char *name) {
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
  if (x && !isSettingUpInternalState()) {
    InitGuard initGuard;
    libmemintercept::MemCallback *cb = &startCallback;
    do {
      cb->memoryAllocated(x, size);
      cb = cb->getNext();
    } while (cb);
  }
  return x;
}

void *calloc(size_t num, size_t size) {
  std::lock_guard guard(internalStateMutex);

  LOAD_REAL_IMPL(calloc);
  void *x = real_calloc(num, size);
  if (x && !isSettingUpInternalState()) {
    InitGuard initGuard;
    libmemintercept::MemCallback *cb = &startCallback;
    do {
      cb->memoryAllocated(x, num * size);
      cb = cb->getNext();
    } while (cb);
  }
  return x;
}

void *realloc(void *p, size_t size) {
  std::lock_guard guard(internalStateMutex);

  LOAD_REAL_IMPL(realloc);
  void *x = real_realloc(p, size);
  if (x && !isSettingUpInternalState()) {
    InitGuard initGuard;
    libmemintercept::MemCallback *cb = &startCallback;
    do {
      cb->memoryReallocated(p, x, size);
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
