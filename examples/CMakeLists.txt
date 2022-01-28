
add_library(printFrees SHARED printFrees.cpp)
target_link_libraries(printFrees PUBLIC libmemintercept)

add_library(pointerToAllocMap SHARED pointerToAllocMap.cpp)
target_link_libraries(pointerToAllocMap PUBLIC libmemintercept)
