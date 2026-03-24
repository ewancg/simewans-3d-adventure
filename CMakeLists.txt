cmake_minimum_required(VERSION 4.1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(PROJECT_NAME "simewan-3d-adventure")
project(
  "${PROJECT_NAME}"
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
    ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

option(BUILD_TESTS "Build all test targets from src/ and tests/" OFF)
option(ENABLE_IPO "Enable IPO/LTO" "")

function(compile_platform_shaders)
  if(WIN32)
    set(SHADER_TARGET_FORMATS dxbc dxbc-asm dxil dxil-asm spirv spirv-asm)
  elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(SHADER_TARGET_FORMATS metal metallib metallib-asm)
  else()
    set(SHADER_TARGET_FORMATS spirv spirv-asm)
  endif()

  # Build neural.slang-module and output to the output directory
  set(SHADER_SOURCES vertex.slang fragment.slang pixel.slang)
  foreach(FORMAT ${SHADER_TARGET_FORMATS})
    foreach(SOURCE ${SHADER_SOURCES})
      get_filename_component(SOURCE_BASENAME "${SOURCE}" NAME)
      add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/shaders"
        COMMAND ${SLANG_COMPILER} ${SHADER_DEFINES} -target "${FORMAT}"
                "${SOURCE}" -o "${SOURCE_BASENAME}"
        DEPENDS ${SHADER_SOURCES} ${SLANG_COMPILER_DEPENDENCY}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        VERBATIM)
      add_custom_target(shader-target ALL DEPENDS ${SOURCES})
      set_target_properties(shader-target PROPERTIES FOLDER shaders)
    endforeach()
  endforeach()
endfunction()

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
  if(CMAKE_BUILD_TYPE IN_LIST DEBUG_CONFIGS)
    if(SANITIZERS_ENABLED)
      if("" STREQUAL "${OPTIMIZATION_LEVEL}")
        set(OPTIMIZATION_LEVEL "-O1")
      endif()
    else()
      set(OPTIMIZATION_LEVEL "-O0")
    endif()
    list(APPEND DEFINES APPLICATION_DEBUGGING AUDIO_DEBUGGING
         GRAPHICS_DEBUGGING WINDOW_DEBUGGING)
  else()
    if("" STREQUAL "${OPTIMIZATION_LEVEL}")
      set(OPTIMIZATION_LEVEL "-O3")
    endif()
    if("" STREQUAL "${ENABLE_IPO}")
      set(ENABLE_IPO ON)
    endif()
  endif()

  if(NOT "" STREQUAL "${ENABLE_IPO}" AND NOT "OFF" STREQUAL ${ENABLE_IPO})
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
  set(glaze_ENABLE_REFLECTION26 ON)
  target_link_libraries(
    "${TARGET}"
    PRIVATE #
            ${ARG_LIBRARIES} #
            asound
            jack
            pulse
            glaze::glaze
            frozen::frozen
            SDL3::Headers
            SDL3::SDL3
            PkgConfig::miniaudio)
  # --- Set #defines
  target_compile_definitions("${TARGET}" PRIVATE #
                                                 ${ARG_DEFINES})

  # --- Set non-define compile flags
  set(FLAGS ${ARG_FLAGS} "${OPTIMIZATION_LEVEL}" "-march=native"
      # "-freflection" # GCC 16 not available in nixpkgs yet
  )
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
find_package(glaze REQUIRED CONFIG)
cmake_pkg_config(IMPORT miniaudio "0.11.24" REQUIRED)

if(${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)
else()
  create_main_executable("${PROJECT_NAME}")
  install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
endif()
