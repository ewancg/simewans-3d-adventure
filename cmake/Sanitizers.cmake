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
    set_property(CACHE "${SANITIZER}" PROPERTY VALUE ON)
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
function(add_flags LIST FLAGS)
  cmake_parse_arguments(AF "GCC_SKIP" "" "GCC_FLAGS" ${ARGN})
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(AF_GCC_SKIP)
      return()
    elseif(AF_GCC_FLAGS)
      list(APPEND ${LIST} ${AF_GCC_FLAGS})
    else()
      list(APPEND ${LIST} ${FLAGS})
    endif()
  else()
    list(APPEND ${LIST} ${FLAGS})
  endif()
  set(${LIST}
      "${${LIST}}"
      PARENT_SCOPE)
endfunction()

function(apply_sanitizers TARGET)
  set(SANITIZER_FLAGS "")
  set(SANITIZER_GCC_LIBS "")

  if(SANITIZER_BACKTRACE_FLAGS)
    add_flags(SANITIZER_FLAGS
              "-fno-omit-frame-pointer;-fno-optimize-sibling-calls")
  endif()
  if(SANITIZER_RT_WARNING_FLAGS)
    add_flags(SANITIZER_FLAGS "-g")
  endif()
  if(ADDRESS_SAN)
    add_flags(SANITIZER_FLAGS "-fsanitize=address")
  endif()
  if(THREAD_SAN)
    add_flags(SANITIZER_FLAGS "-fsanitize=thread")
  endif()
  if(MEMORY_SAN)
    add_flags(SANITIZER_FLAGS "-fsanitize=memory")
  endif()
  if(MEMORY_SAN_EX)
    add_flags(SANITIZER_FLAGS "-fsanitize-memory-track-origins")
  endif()
  if(UB_SAN)
    add_flags(SANITIZER_FLAGS
              "-fsanitize=undefined,undefined-strip-path-components=-2"
              GCC_FLAGS "-fsanitize=undefined")
    add_flags(SANITIZER_GCC_LIBS "ubsan")
  endif()
  if(UB_SAN_EX)
    add_flags(SANITIZER_FLAGS
              "-fsanitize=integer,implicit-conversion,nullability" GCC_SKIP)
  endif()
  if(LEAK_SAN)
    add_flags(SANITIZER_FLAGS "-fsanitize=leak")
  endif()
  if(TYPE_SAN)
    add_flags(SANITIZER_FLAGS "-fsanitize=type" GCC_SKIP)
  endif()
  if(RT_SAN)
    add_flags(SANITIZER_FLAGS "-fsanitize=realtime")
  endif()

  target_compile_options("${TARGET}" PRIVATE ${SANITIZER_FLAGS})
  target_link_options("${TARGET}" PRIVATE ${SANITIZER_FLAGS})
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND SANITIZER_GCC_LIBS)
    target_link_libraries("${TARGET}" PRIVATE ${SANITIZER_GCC_LIBS})
  endif()
endfunction()

macro(check_sanitizers_enabled)
  foreach(
    VAR
    ADDRESS_SAN
    THREAD_SAN
    MEMORY_SAN
    MEMORY_SAN_EX
    UB_SAN
    UB_SAN_EX
    UB_SAN_MIN_RT
    RT_SAN
    LEAK_SAN
    TYPE_SAN
    SAN_PRESET_ASAN
    SAN_PRESET_TSAN
    SAN_PRESET_MSAN
    SAN_PRESET_UB_SAN_PROD)
    if(${VAR})
      set(SANITIZERS_ENABLED true)
      return(PROPAGATE SANITIZERS_ENABLED)
    endif()
  endforeach()
endmacro()
