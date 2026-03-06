cmake_minimum_required(VERSION 4.1)

set(PROJECT_NAME "simewan-3d-adventure")
project(
  "${PROJECT_NAME}"
  VERSION 1.0
  LANGUAGES CXX)

option(BUILD_TESTS "Build all test targets from src/ and tests/" OFF)

option(BUILD_ASAN "Build with the address sanitizer" OFF)
option(BUILD_ASAN_EX "Build with backtrace-mangling optimizations disabled" OFF)
option(BUILD_TSAN "Build with the thread sanitizer" OFF) # TODO: look up complementary/extension flags for tsan (and all after here)
option(BUILD_MSAN "Build with the memory sanitizer" OFF)
option(BUILD_UBSAN "Build with undefined behavior sanitizer" OFF)
option(BUILD_LSAN "Build with the memory leak sanitizer" OFF)
option(BUILD_ISAN "Build with the integer overflow sanitizer" OFF)

# Practical presets for QA (mutually exclusive sanitizer combinations maximizing other sanitizers)
option(BUILD_SANITIZERS_1 "Build with the address, undefined behavior, memory leak, and integer overflow sanitizers" OFF)
option(BUILD_SANITIZERS_2 "Build with the thread, undefined behavior, memory leak, and integer overflow sanitizers" OFF)
option(BUILD_SANITIZERS_3 "Build with the memory, undefined behavior, memory leak, and integer overflow sanitizers" OFF)

if(BUILD_SANITIZERS_1)
    set(BUILD_ASAN ON CACHE FORCE)
    set(BUILD_ASAN_EX ON CACHE FORCE)
    set(BUILD_UBSAN ON CACHE FORCE)
    set(BUILD_LSAN ON CACHE FORCE)
    set(BUILD_ISAN ON CACHE FORCE)
elseif(BUILD_SANITIZERS_2)
    set(BUILD_TSAN ON CACHE FORCE)
    set(BUILD_UBSAN ON CACHE FORCE)
    set(BUILD_LSAN ON CACHE FORCE)
    set(BUILD_ISAN ON CACHE FORCE)
elseif(BUILD_SANITIZERS_3)
    set(BUILD_MSAN ON CACHE FORCE)
    set(BUILD_UBSAN ON CACHE FORCE)
    set(BUILD_LSAN ON CACHE FORCE)
    set(BUILD_ISAN ON CACHE FORCE)
endif()

# https://clang.llvm.org/docs/LeakSanitizer.html#usage
# Force clang as linker
if(BUILD_LSAN)
    set(LINKER_FLAGS "<FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
    set(CMAKE_CXX_LINK_EXECUTABLE "clang++ ${LINKER_FLAGS}")
    set(CMAKE_C_LINK_EXECUTABLE "clang ${LINKER_FLAGS}")
endif()

function(create_main_executable TARGET)
  # Parse arguments
  if("${TARGET}" STREQUAL "")
    message(FATAL_ERROR "No TARGET provided to create_main_executable")
  endif()

  include(CMakeParseArguments)
  cmake_parse_arguments(
    ARG # output var prefix
    "" # boolean arguments
    "" # "mono-valued" arguments
    "SOURCES;INCLUDE_DIRS;LIBRARIES;DEFINES;FLAGS" # "multi-valued" arguments (list)
    ${ARGN})

  unset(ROOT_SOURCES)
  foreach(SRC   main.cpp
                input.cpp)
    list(APPEND ROOT_SOURCES "${CMAKE_SOURCE_DIR}/src/${SRC}")
  endforeach()

  get_property(DEBUG_CONFIGS GLOBAL PROPERTY DEBUG_CONFIGURATIONS)
  if(CMAKE_BUILD_TYPE IN_LIST DEBUG_CONFIGS)
    set(OPTIMIZATION_LEVEL "-O1")
  endif()

  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  set(TARGET_SOURCES ${ARG_SOURCES} ${ROOT_SOURCES})
  set(LIBRARIES ${ARG_LIBRARIES}
      SDL3::Headers
      SDL3::SDL3
  )
  set(DEFINES ${ARG_DEFINES})
  set(INCLUDE_DIRS ${ARG_INCLUDE_DIRS}
      "${CMAKE_SOURCE_DIR}/include"
      "${CMAKE_SOURCE_DIR}/external"
  )
  set(FLAGS ${ARG_FLAGS}
      "${OPTIMIZATION_LEVEL}"
      "$<$<BOOL:${BUILD_ASAN}>:-fsanitize=address>"
      "$<$<BOOL:${BUILD_ASAN_EX}>:-fno-omit-frame-pointer -fno-optimize-sibling-calls>"
      "$<$<BOOL:${BUILD_TSAN}>:-fsanitize=thread>"
      "$<$<BOOL:${BUILD_MSAN}>:-fsanitize=memory>"
      "$<$<BOOL:${BUILD_UBSAN}>:-fsanitize=undefined>"
      "$<$<BOOL:${BUILD_LSAN}>:-fsanitize=leak>"
      "$<$<BOOL:${BUILD_ISAN}>:-fsanitize=integer>"
  )
  # March 2026 (twenty six!!!); Certain C++23 features are experimental in MinGW
  if(WIN32 AND (MSYS OR MINGW))
    list(APPEND LIBRARIES "stdc++exp")
  endif()

  add_executable("${TARGET}" ${TARGET_SOURCES})
  target_include_directories("${TARGET}" PRIVATE ${INCLUDE_DIRS})
  target_link_libraries("${TARGET}" PRIVATE ${LIBRARIES})
  target_compile_definitions("${TARGET}" PRIVATE ${DEFINES})
  target_compile_options("${TARGET}" PRIVATE ${FLAGS})
  set_target_properties(
    "${TARGET}" PROPERTIES WIN32_EXECUTABLE ON #
                           MACOSX_BUNDLE ON #
  )
  target_precompile_headers("${TARGET}" PUBLIC "${CMAKE_SOURCE_DIR}/src/common.h")
endfunction()

# find_package and such
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
target_link_libraries("${TARGET}" PRIVATE SDL3)

if(${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)
else()
  create_main_executable("${PROJECT_NAME}")
  install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
endif()
