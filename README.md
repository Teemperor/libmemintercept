# libmemintercept
Library for intercepting memory allocation functions (malloc, free, ...).

## Build instructions

```bash
# Create build directory.
mkdir build ; cd build
# Run CMake to setup build directory.
cmake .. -GNinja
# Build.
ninja
# Build and run tests.
ninja check
```

## How to use

See the `examples/` directory for compiling examples.

A minimal example is:

```cpp
#include "libmemintercept.h"
#include <cstdio>

// Your backend. See Memcallback's documentation for available hooks.
struct PrintFrees : libmemintercept::MemCallback {
  void memoryAllocated(void *addr, std::size_t size) {}

  void memoryReallocated(void *old_addr, void *new_addr,
                                 std::size_t new_size) override {}

  void memoryFreed(void *addr) override {
    fprintf(stderr, "free(%p)\n", addr);
  }
};

// Sets up hook.
LIBMEMINTERCEPT_ADD_CB(PrintFrees)
```

This needs to link against `libmemintercept` to work and should be compiled into a shared objetc.

To actually intercept a process do:

```bash
$ LD_PRELOAD=path/to/your.so binary_you_want_to_intercept
```
