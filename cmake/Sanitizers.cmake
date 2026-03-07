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

function(populate_sanitizer_flags LIST)
  list(
    APPEND
    ${LIST}
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
endfunction()
