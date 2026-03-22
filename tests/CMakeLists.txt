find_package(Catch2 3 REQUIRED)
include(Catch)

# Enable runtime asserts in release mode
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE
               "${CMAKE_CXX_FLAGS_RELEASE}")

# Stops excessive formatting on test failure
set(CATCH2_ARGS --reporter compact --verbosity quiet)

set(TESTS_PROJECT_NAME "${PROJECT_NAME}-tests")
create_main_executable(
  "${TESTS_PROJECT_NAME}"
  LIBRARIES
  Catch2::Catch2WithMain
  DEFINES
  UNIT_TESTING
  frozen.tests
  MINIAUDIO_BUILD_TESTS
  SDL_TESTS
  SDL_TEST_LIBRARY)
catch_discover_tests("${TESTS_PROJECT_NAME}" EXTRA_ARGS ${CATCH2_ARGS}
                     TEST_PREFIX "inline:")

# Adds a standalone executable with main() as a test for CTest
function(add_integration_test_frodir NAME SRC_DIR)
  set(TARGET "itest-${NAME}")
  add_executable("${TARGET}" "${SRC_DIR}/main.cpp")
  target_include_directories("${TARGET}" PRIVATE "${CMAKE_SOURCE_DIR}/include")
  add_test(NAME "integration:${NAME}" COMMAND "${TARGET}")
endfunction()

# Sources any amount of Catch2 unit tests from one file into CTest
function(add_unit_tests_frofile NAME SRC_FILES)
  set(TARGET "utest-${NAME}")
  add_executable("${TARGET}" ${SRC_FILES})
  target_include_directories("${TARGET}" PRIVATE "${CMAKE_SOURCE_DIR}/include")
  target_link_libraries("${TARGET}" PRIVATE Catch2::Catch2WithMain)
  catch_discover_tests("${TARGET}" EXTRA_ARGS ${CATCH2_ARGS} TEST_PREFIX
                       "unit:")
endfunction()

file(
  GLOB FILES
  LIST_DIRECTORIES true
  "*")
foreach(item ${FILES})
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  get_filename_component(TEST_NAME "${item}" NAME_WE)
  if(IS_DIRECTORY "${item}")
    add_integration_test_frodir("${TEST_NAME}" "${item}")
  else()
    get_filename_component(EXTENSION "${item}" LAST_EXT)
    if("${EXTENSION}" STREQUAL ".cpp")
      add_unit_tests_frofile("${TEST_NAME}" "${item}")
    endif()
  endif()
endforeach()
