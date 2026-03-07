cmake_minimum_required(VERSION 4.1)

set(PROJECT_NAME "simewan-3d-adventure")
project(
  "${PROJECT_NAME}"
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build all test targets from src/ and tests/" OFF)

# For ASan, MSan, TySan: get perfect stack traces by disabling inlining and tail
# call elimination
option(SANITIZER_BACKTRACE_FLAGS
       "Build with backtrace-mangling optimizations disabled" OFF)
# For TSan: use -g to get file names and line numbers in the warning messages.
option(SANITIZER_RT_WARNING_FLAGS
       "Build with file names & line numbers in warnings" OFF)

# https://clang.llvm.org/docs/AddressSanitizer.html - highlights: Typical
# slowdown introduced by AddressSanitizer is 2x. Use clang for the final link
# step. -Wl,-z,defs may cause link errors when linking shared libraries.
option(ADDRESS_SAN "Build with the address sanitizer" OFF)

# https://clang.llvm.org/docs/ThreadSanitizer.html - highlights: Typical memory
# overhead introduced by ThreadSanitizer is about 5x-10x. Libc/libstdc++ static
# linking is not supported. Non-position-independent executables are not
# supported.
option(THREAD_SAN "Build with the thread sanitizer" OFF)

# https://clang.llvm.org/docs/MemorySanitizer.html - highlights: Typical
# slowdown introduced by MemorySanitizer is 3x. MemorySanitizer uses 2x more
# real memory than a native run, 3x with origin tracking. Static linking is not
# supported. MemorySanitizer requires that all program code is instrumented.
option(MEMORY_SAN "Build with the memory sanitizer" OFF)
# MemorySanitizer can track origins of uninitialized values, similar to
# Valgrind’s –track-origins option.
option(MEMORY_SAN_EX "Build with memory allocation origin tracking" OFF)

# https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html - highlights:
# UBSan modifies the program at compile-time to catch various kinds of undefined
# behavior during program execution.
option(UB_SAN "Build with undefined behavior sanitizer" OFF)
option(
  UB_SAN_EX
  "Build with integer overflow, harmful implicit cast, and nullability violation detection"
  OFF)
option(UB_SAN_MIN_RT
       "Build with the undefined behavior sanitizer's production-grade runtime"
       OFF)

# https://clang.llvm.org/docs/RealtimeSanitizer.html - highlights: Performance
# benefits from annotating known blocking functions with [[clang::blocking]]
# Annotate real-time thread entry points with [[clang::nonblocking]]
option(RT_SAN "Build with real-time safety sanitizer" OFF)

# https://clang.llvm.org/docs/LeakSanitizer.html
option(LEAK_SAN "Build with the memory leak sanitizer" OFF)
# https://clang.llvm.org/docs/TypeSanitizer.html - highlights: It provides
# `-fno-strict-aliasing`-type checks at runtime, avoiding the need for the flag
# in production (improving performance). Typical memory overhead introduced by
# TypeSanitizer is about 8x. Runtime slowdown varies greatly depending on how
# often the instrumented code relies on type aliasing. In the best case slowdown
# is 2x-3x.
option(TYPE_SAN "Build with the type sanitizer" OFF)

# Practical presets for QA (mutually exclusive sanitizer combinations maximizing
# other sanitizers)
option(
  SAN_PRESET_ASAN
  "Build with the address, undefined behavior, memory leak, and integer overflow sanitizers"
  OFF)
option(
  SAN_PRESET_TSAN
  "Build with the thread, undefined behavior, memory leak, and integer overflow sanitizers"
  OFF)
option(
  SAN_PRESET_MSAN
  "Build with the memory, undefined behavior, memory leak, and integer overflow sanitizers"
  OFF)
option(
  SAN_PRESET_UB_SAN_PROD
  "Build with the undefined behavior (and its minimal runtime), memory leak, and integer overflow sanitizers"
  OFF)

include(CMakeDependentOption)
function(enable_sanitizers)
  foreach(SANITIZER ${ARGN})
    set("${SANITIZER}" ON FORCE)
  endforeach()
endfunction()
if(SAN_PRESET_ASAN)
  enable_sanitizers(ADDRESS_SAN SANITIZER_BACKTRACE_FLAGS UB_SAN UB_SAN_EX
                    LEAK_SAN TYPE_SAN)
elseif(SAN_PRESET_TSAN)
  enable_sanitizers(THREAD_SAN SANITIZER_RT_WARNING_FLAGS UB_SAN UB_SAN_EX
                    LEAK_SAN TYPE_SAN)
elseif(SAN_PRESET_MSAN)
  enable_sanitizers(
    MEMORY_SAN
    MEMORY_SAN_EX
    SANITIZER_BACKTRACE_FLAGS
    UB_SAN
    UB_SAN_EX
    LEAK_SAN
    TYPE_SAN)
elseif(SAN_PRESET_UB_SAN_PROD)
  enable_sanitizers(UB_SAN UB_SAN_EX UB_SAN_MIN_RT LEAK_SAN TYPE_SAN)
endif()

# Force clang as linker (required by any sanitizers with runtime libraries)
if(ADDRESS_SAN
   OR MEMORY_SAN
   OR UB_SAN
   OR LEAK_SAN)
  set(LINKER_FLAGS
      "<FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
  )
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
    "SOURCES;INCLUDE_DIRS;LIBRARIES;DEFINES;FLAGS" # "multi-valued" arguments
                                                   # (list)
    ${ARGN})

  unset(ROOT_SOURCES)
  foreach(SRC main.cpp input.cpp graphics.cpp)
    list(APPEND ROOT_SOURCES "${CMAKE_SOURCE_DIR}/src/${SRC}")
  endforeach()

  get_property(DEBUG_CONFIGS GLOBAL PROPERTY DEBUG_CONFIGURATIONS)
  if(CMAKE_BUILD_TYPE IN_LIST DEBUG_CONFIGS)
    set(OPTIMIZATION_LEVEL "-O1")
  endif()

  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(TARGET_SOURCES ${ARG_SOURCES} ${ROOT_SOURCES})
  set(LIBRARIES ${ARG_LIBRARIES} SDL3::Headers SDL3::SDL3)
  set(DEFINES ${ARG_DEFINES})
  set(INCLUDE_DIRS ${ARG_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/include"
                   "${CMAKE_SOURCE_DIR}/external")
  set(FLAGS
      ${ARG_FLAGS}
      "${OPTIMIZATION_LEVEL}"
      "$<$<BOOL:${SANITIZER_BACKTRACE_FLAGS}>:-fno-omit-frame-pointer -fno-optimize-sibling-calls>"
      "$<$<BOOL:${SANITIZER_RT_WARNING_FLAGS}>:-g>"
      "$<$<BOOL:${ADDRESS_SAN}>:-fsanitize=address>"
      "$<$<BOOL:${THREAD_SAN}>:-fsanitize=thread>"
      "$<$<BOOL:${MEMORY_SAN}>:-fsanitize=memory>"
      "$<$<BOOL:${MEMORY_SAN_EX}>:-fsanitize-memory-track-origins>"
      "$<$<BOOL:${UB_SAN}>:-fsanitize=undefined,undefined-strip-path-components=-2>"
      "$<$<BOOL:${UB_SAN_EX}>:-fsanitize=integer,implicit-conversion,nullability>"
      "$<$<BOOL:${LEAK_SAN}>:-fsanitize=leak>"
      "$<$<BOOL:${TYPE_SAN}>:-fsanitize=type>"
      "$<$<BOOL:${RT_SAN}>:-fsanitize=realtime>")
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
  target_precompile_headers("${TARGET}" PUBLIC
                            "${CMAKE_SOURCE_DIR}/src/common.h")
endfunction()

# find_package and such
find_package(SDL3 REQUIRED)

if(${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests)
else()
  create_main_executable("${PROJECT_NAME}")
  install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
endif()
