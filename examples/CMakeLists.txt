
add_library(printFrees SHARED printFrees.cpp)
target_link_libraries(printFrees PUBLIC libmemintercept)
