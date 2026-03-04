cmake_minimum_required(VERSION 4.1)

set(PROJECT_NAME "simewan-3d-adventure")
project(
  "${PROJECT_NAME}"
  VERSION 1.0
  LANGUAGES CXX)

option(BUILD_TESTS "Build unit and integration tests in tests/" OFF)

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
    "SOURCES;INCLUDE_DIRS;LIBRARIES;DEFINES" # "multi-valued" arguments (list)
    ${ARGN})

  unset(ROOT_SOURCES)
  foreach(SRC main.cpp)
    list(APPEND ROOT_SOURCES "${CMAKE_SOURCE_DIR}/src/${SRC}")
  endforeach()
  set(TARGET_SOURCES ${ARG_SOURCES} ${ROOT_SOURCES})

  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  set(LIBRARIES ${ARG_LIBRARIES})
  set(DEFINES ${ARG_DEFINES})
  set(INCLUDE_DIRS ${ARG_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/include"
                   "${CMAKE_SOURCE_DIR}/external")

  # March 2026 (twenty six!!!); Certain C++23 features are experimental in MinGW
  if(WIN32 AND (MSYS OR MINGW))
    list(APPEND LIBRARIES "stdc++exp")
  endif()

  add_executable("${TARGET}" ${TARGET_SOURCES})
  target_compile_definitions("${TARGET}" PRIVATE ${DEFINES})
  target_link_libraries("${TARGET}" PRIVATE ${LIBRARIES})
  target_include_directories("${TARGET}" PRIVATE ${INCLUDE_DIRS})
  set_target_properties(
    "${TARGET}" PROPERTIES WIN32_EXECUTABLE ON #
                           MACOSX_BUNDLE ON #
  )
endfunction()

# find_package and such

if(${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)
else()
  create_main_executable("${PROJECT_NAME}")
  install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
endif()
