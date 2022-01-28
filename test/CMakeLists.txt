set(TEST_DEPENDS "")

set(ALL_EXAMPLES printFrees pointerToAllocMap)

set(TEST_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run.py")

set(TEST_ENV_INITS )
foreach(EXAMPLE ${ALL_EXAMPLES})
  set(TEST_ENV_INITS "${TEST_ENV_INITS}:${EXAMPLE}=$<TARGET_FILE:${EXAMPLE}>")
endforeach()

function(add_preload_test)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs EXTRA_LIB)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}"
                          "${multiValueArgs}" ${ARGN} )

    set(EXE_NAME "test_${ARG_NAME}")
    add_executable("${EXE_NAME}" "${ARG_NAME}.cpp" ${ARG_EXTRA_SOURCES})
    if (ARG_EXTRA_LIB)
      add_library("${EXE_NAME}_extralib" SHARED ${ARG_EXTRA_LIB})
      target_link_libraries("${EXE_NAME}" PUBLIC "${EXE_NAME}_extralib")
    endif()
    set(TEST_DEPENDS "${TEST_DEPENDS};${EXE_NAME}" PARENT_SCOPE)

    set(TEST_SH "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_NAME}.sh")
    add_test(NAME "${ARG_NAME}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      COMMAND "${TEST_RUNNER}" ${TEST_ENV_INITS} "${TEST_SH}"
      $<TARGET_FILE:${EXE_NAME}> "${CMAKE_CURRENT_BINARY_DIR}/${ARG_NAME}.output")
endfunction()

add_preload_test(NAME SanityCheck)
add_preload_test(NAME CheckMalloc)
add_preload_test(NAME AllocInfo EXTRA_LIB AllocInfoLib.cpp)

message(STATUS "DEPS: ${TEST_DEPENDS}")
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose
                  DEPENDS ${ALL_EXAMPLES} ${TEST_DEPENDS})
