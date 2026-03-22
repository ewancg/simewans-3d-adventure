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
option(ENABLE_IPO "Enable IPO/LTO")

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
  check_sanitizers_enabled()
  if(SANITIZERS_ENABLED
     AND NOT OPTIMIZATION_LEVEL
     AND CMAKE_BUILD_TYPE IN_LIST DEBUG_CONFIGS)
    set(OPTIMIZATION_LEVEL "-O1")
  endif()

  if(ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
      set_property(TARGET "${TARGET}" PROPERTY INTERPROCEDURAL_OPTIMIZATION
                                               TRUE)
    else()
      message(WARNING "IPO is not supported: ${output}")
    endif()
  endif()

  # --- Create executable
  unset(ROOT_SOURCES)
  foreach(SRC #
          main.cpp Application.cpp Audio.cpp Graphics.cpp Input.cpp Window.cpp)
    list(APPEND ROOT_SOURCES "${CMAKE_SOURCE_DIR}/src/${SRC}")
  endforeach()
  add_executable("${TARGET}" ${ARG_SOURCES} ${ROOT_SOURCES})

  # --- Populate include directories
  target_include_directories("${TARGET}" PRIVATE "${CMAKE_SOURCE_DIR}/include")

  # --- Link libraries

  # March 2026 certain C++23 features are experimental in MinGW
  if(WIN32 AND (MSYS OR MINGW))
    list(APPEND ARG_LIBRARIES "stdc++exp")
  endif()
  target_link_libraries(
    "${TARGET}"
    PRIVATE #
            ${ARG_LIBRARIES} #
            asound
            jack
            pulse
            PkgConfig::miniaudio
            frozen::frozen # constexpr STL-mimicking types
            SDL3::Headers
            SDL3::SDL3)
  # --- Set #defines
  target_compile_definitions("${TARGET}" PRIVATE #
                                                 ${ARG_DEFINES})

  # --- Set non-define compile flags
  set(FLAGS ${ARG_FLAGS} "${OPTIMIZATION_LEVEL}" "-march=native")
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
find_package(SDL3 REQUIRED CONFIG)
find_package(frozen REQUIRED CONFIG)
cmake_pkg_config(IMPORT miniaudio "0.11.24" REQUIRED)

if(${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)
else()
  create_main_executable("${PROJECT_NAME}")
  install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
endif()
