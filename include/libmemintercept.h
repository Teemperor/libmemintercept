#ifndef LIBMEMINTERCEPT_H
#define LIBMEMINTERCEPT_H

#include <cstdint>

namespace libmemintercept {
/// The base class for a malloc/free callback handler.
///
/// 1. Can have state.
/// 2. All APIs are guaranteed to be only accessed by a single thread at once.
/// 3. Its destructor is not guaranteed to be called.
class MemCallback {
  /// Points to the next MemCallback.
  MemCallback *next = nullptr;

public:
  /// Internal API. Adds the given callback to the callback list.
  void chainUpNext(MemCallback *n) {
    if (next != nullptr) {
      next->chainUpNext(n);
      return;
    }
    next = n;
  }

  /// Internal API. Returns the next linked MemCallback.
  MemCallback *getNext() { return next; }

  /// Called after (potentially zero initialized) memory is allocated.
  ///
  /// Callback for both malloc and calloc.
  ///
  /// \param addr The address where the memory was allocated.
  /// \param size The size in bytes of the allocated memory.
  virtual void memoryAllocated(void *addr, std::size_t size) {
    (void)addr;
    (void)size;
  }

  /// Called after memory is reallocated (via e.g. realloc).
  ///
  /// \param old_addr The previous address (that is now invalid).
  /// \param new_addr The new address where the memory resides.
  /// \param new_size The new size of the allocated area.
  virtual void memoryReallocated(void *old_addr, void *new_addr,
                                 std::size_t new_size) {
    (void)old_addr;
    (void)new_addr;
    (void)new_size;
  }

  /// Called before memory is free'd.
  ///
  /// \param addr The address of the free'd memory. The memory is still valid.
  virtual void memoryFreed(void *addr) { (void)addr; }
};

/// Installs the MemCallback created by the given callback.
/// The given callback is allowed to use malloc/free/etc. and those callbacks
/// are *not* observed by other callbacks.
void createAndInstallMemCallback(MemCallback *(*createCallback)());
} // namespace libmemintercept

// Has to be passed a class name with a default constructor. Creates an instance
// of that class and hooks it up as a callback for malloc/free/etc.
// Macro has to be called on file-scope.
#define LIBMEMINTERCEPT_ADD_CB(CBNAME)                                         \
  namespace {                                                                  \
  static libmemintercept::MemCallback *createCB##CBNAME() {                    \
    return new CBNAME();                                                       \
  }                                                                            \
  struct StartupCB##CBNAME {                                                   \
    StartupCB##CBNAME() {                                                      \
      libmemintercept::createAndInstallMemCallback(&createCB##CBNAME);         \
    }                                                                          \
  };                                                                           \
  }                                                                            \
  StartupCB##CBNAME libmemintercept_startup##CBNAME;

#endif // LIBMEMINTERCEPT_H
