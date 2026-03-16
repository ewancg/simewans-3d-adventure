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
  # Parse arguments
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

  unset(ROOT_SOURCES)
  foreach(SRC main.cpp Subsystem.cpp Audio.cpp Graphics.cpp Input.cpp
              Window.cpp)
    list(APPEND ROOT_SOURCES "${CMAKE_SOURCE_DIR}/src/${SRC}")
  endforeach()

  get_property(DEBUG_CONFIGS GLOBAL PROPERTY DEBUG_CONFIGURATIONS)
  if(CMAKE_BUILD_TYPE IN_LIST DEBUG_CONFIGS)
    set(OPTIMIZATION_LEVEL "-O1")
  endif()

  set(TARGET_SOURCES ${ARG_SOURCES} ${ROOT_SOURCES})
  set(LIBRARIES ${ARG_LIBRARIES} SDL3::Headers SDL3::SDL3)
  set(DEFINES ${ARG_DEFINES})
  set(INCLUDE_DIRS ${ARG_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/include"
                   "${CMAKE_SOURCE_DIR}/external")
  set(FLAGS ${ARG_FLAGS} "${OPTIMIZATION_LEVEL}")
  # March 2026 (twenty six!!!); Certain C++23 features are experimental in MinGW
  if(WIN32 AND (MSYS OR MINGW))
    list(APPEND LIBRARIES "stdc++exp")
  endif()

  add_executable("${TARGET}" ${TARGET_SOURCES})
  target_include_directories("${TARGET}" PRIVATE ${INCLUDE_DIRS})
  target_link_libraries("${TARGET}" PRIVATE ${LIBRARIES})
  target_compile_definitions("${TARGET}" PRIVATE ${DEFINES})
  target_compile_options("${TARGET}" PRIVATE ${FLAGS})
  target_link_options("${TARGET}" PRIVATE ${FLAGS})
  apply_sanitizers("${TARGET}")
  set_target_properties(
    "${TARGET}" PROPERTIES WIN32_EXECUTABLE ON #
                           MACOSX_BUNDLE ON #
  )
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
