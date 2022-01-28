#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <mutex>

#include "libmemintercept.h"

namespace {

/// Keeps track if we are in internal libmemintercept code.
/// 0 if we are inside libmemintercept code, otherwise > 0.
/// Used to prevent infinite recursion when doing e.g. malloc inside the malloc
/// handler.
static unsigned settingUpInternalState = 0;
static bool isSettingUpInternalState() { return settingUpInternalState != 0; }

/// Disables interposing memory functions while in scope.
struct DisableInterposition {
  DisableInterposition() { settingUpInternalState++; }
  ~DisableInterposition() { settingUpInternalState--; }
};


struct StartMemCallback : public libmemintercept::MemCallback {};
/// This is the initial MemCallback that does nothing but start the linked list
/// of MemCallback instances.
static StartMemCallback startCallback;
} // namespace

namespace libmemintercept {
void createAndInstallMemCallback(MemCallback *(*createCallback)()) {
  DisableInterposition guard;
  MemCallback *c = createCallback();
  startCallback.chainUpNext(c);
}
} // namespace libmemintercept

// List of the real libc implementations of memory functions.
// These are lazily dlsym'd on first use.
static void *(*real_malloc)(size_t) = nullptr;
static void *(*real_calloc)(size_t, size_t) = nullptr;
static void *(*real_realloc)(void *, size_t) = nullptr;
static void (*real_free)(void *) = nullptr;

/// Takes a function pointer reference and a name to dlsym, then looks up the
/// symbol if the pointer is not null.
template <typename T> static void loadRealImplIfNeeded(T &t, const char *name) {
  if (t)
    return;

  t = reinterpret_cast<T>(dlsym(RTLD_NEXT, name));
  if (!t) {
    // FIXME: Put an error message here that doesn't require allocation.
    abort();
  }
}

#define LOAD_REAL_IMPL(NAME) loadRealImplIfNeeded(real_##NAME, #NAME)

/// Taken if we are in an interposed function. Prevents that the clients
/// have to deal with threading.
static std::recursive_mutex internalStateMutex;

extern "C" {
void *malloc(size_t size) {
  std::lock_guard guard(internalStateMutex);

  LOAD_REAL_IMPL(malloc);
  void *x = real_malloc(size);
  if (x && !isSettingUpInternalState()) {
    DisableInterposition initGuard;
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
    DisableInterposition initGuard;
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
    DisableInterposition initGuard;
    libmemintercept::MemCallback *cb = &startCallback;
    do {
      cb->memoryReallocated(p, x, size);
      cb = cb->getNext();
    } while (cb);
  }
  return x;
}

void free(void *p) {
  // Dumb optimization that simplifies client code. free(0) is a no-op.
  if (p == nullptr)
    return;

  std::lock_guard guard(internalStateMutex);
  LOAD_REAL_IMPL(free);
  if (!isSettingUpInternalState()) {
    DisableInterposition initGuard;
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
