cmake_minimum_required(VERSION 4.1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(PROJECT_NAME "simewan-3d-adventure")
project(
  "${PROJECT_NAME}"
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
    ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

option(BUILD_TESTS "Build all test targets from src/ and tests/" OFF)

function(create_main_executable TARGET)
  # --- Setup
  if("${TARGET}" STREQUAL "")
    message(FATAL_ERROR "No TARGET provided to create_main_executable")
  endif()

  include(CMakeParseArguments)
  cmake_parse_arguments(
    ARG # output var prefix
    "" # boolean arguments
    "" # "mono-valued" arguments
    "SOURCES;INCLUDE_DIRS;LIBRARIES;DEFINES;FLAGS" # "multi-valued" arguments
                                                   # (list)
    ${ARGN})

  get_property(DEBUG_CONFIGS GLOBAL PROPERTY DEBUG_CONFIGURATIONS)
  if(NOT OPTIMIZATION_LEVEL AND CMAKE_BUILD_TYPE IN_LIST DEBUG_CONFIGS)
    set(OPTIMIZATION_LEVEL "-O1")
  endif()

  # --- Create executable
  unset(ROOT_SOURCES)
  foreach(SRC #
          main.cpp Application.cpp Audio.cpp Graphics.cpp Input.cpp Window.cpp)
    list(APPEND ROOT_SOURCES "${CMAKE_SOURCE_DIR}/src/${SRC}")
  endforeach()
  add_executable("${TARGET}" ${ARG_SOURCES} ${ROOT_SOURCES})

  # --- Populate include directories
  target_include_directories(
    "${TARGET}"
    PRIVATE #
            ${ARG_INCLUDE_DIRS} #
            "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/external")

  # --- Link libraries

  # March 2026 certain C++23 features are experimental in MinGW
  if(WIN32 AND (MSYS OR MINGW))
    list(APPEND ARG_LIBRARIES "stdc++exp")
  endif()
  target_link_libraries(
    "${TARGET}"
    PRIVATE #
            ${ARG_LIBRARIES} #
            SDL3::Headers SDL3::SDL3)
  # --- Set #defines
  target_compile_definitions(
    "${TARGET}" PRIVATE #
                        ${ARG_DEFINES} #
  )

  # --- Set non-define compile flags
  set(FLAGS ${ARG_FLAGS} "${OPTIMIZATION_LEVEL}")
  target_compile_options("${TARGET}" PRIVATE ${FLAGS})
  target_link_options("${TARGET}" PRIVATE ${FLAGS})

  # --- Finalize
  apply_sanitizers("${TARGET}")
  set_target_properties("${TARGET}" PROPERTIES WIN32_EXECUTABLE ON MACOSX_BUNDLE
                                                                   ON)
  target_precompile_headers("${TARGET}" PUBLIC
                            "${CMAKE_SOURCE_DIR}/src/common.h")
endfunction()

include(FetchContent)
include(Sanitizers)
find_package(SDL3 REQUIRED)

if(${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)
else()
  create_main_executable("${PROJECT_NAME}")
  install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
endif()
