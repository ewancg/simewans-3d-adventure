#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-avoid-magic-numbers,
// hicpp-avoid-c-array)
TEST_CASE("arrays and vectors work how i think", "") {
#define CONTENTS {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}
  // C array (sequential and aligned items, verbatim)
  uint32_t array[] = CONTENTS;
  // C++ vector (container implementing a dynamic array)
  auto vector = std::vector CONTENTS;
#undef CONTENTS

  // ----- this section covers how general usage/traversal differs between C arrays and vectors

  // Iterate on items in the C array based on its size
  for (unsigned i{}; i < sizeof(array) / sizeof(unsigned); i++) {
    // Manual index address calculation with pointer arithmetic
    // Explicit dereference means that step can be omitted if traversing an array of pointers
    // *T x + int y = (addr of x) + (sizeof(T) * y)
    REQUIRE(*(array + i) == i + 1);
    // Automatic array indexing (array subscript)
    // Syntactic sugar for the above, always dereferences
    REQUIRE(array[i] == i + 1);

    // Both methods of indexing yield the same address
    REQUIRE(static_cast<void *>(&array[i]) == static_cast<void *>(array + i));

    // This is essentially the complete "API" for C arrays
  }

  // Iterate on items in the vector with STL iterators
  for (const auto &iter : vector) {
    static int last{};
    REQUIRE(last == iter - 1); // validating the iterator is sequential
    last = iter;

    // This method does not give you an index, which can often times be limiting for more complex loops
    // This is how you can access the very first/last items of a STL container, and use them in the absence of an index.
    // The iterators have special for loop syntax, and they beat indices where boundaries are unclear & OOB is a risk
    // STL iterators implement operator*() to return their values, effectively dereferencing (just not a pointer).
    // They also implement operator+() and operator-() to emulate pointer arithmetic
    // begin()/end() can be prefixed with c (to get a constant iterator) and/or r (to get a backwards iterator)
    if (iter == 1) {
      REQUIRE(iter == *vector.begin());
    }
    if (iter == 16) {
      // if the vector is empty, begin == end so subtracting from end & dereferencing is illegal
      std::vector<int> empty{};
      REQUIRE((empty.empty() && empty.begin() == empty.end()));

      // vector.end() represents one past the end, and is used (e.g.) return no value in query functions
      REQUIRE(!vector.empty());
      REQUIRE(iter == *(vector.end() - 1));
    }
  }

  // Another STL iterator idiom, allowing direct access to the iterator object instead of a scoped reference
  auto iter = vector.begin();
  while (iter != vector.end()) {
    static int last{};
    REQUIRE(last == (*iter) - 1);
    last = *iter;

    iter++; // important
  }

  // Iterate on items in the vector with indices
  for (unsigned i{}; i < vector.size(); i++) {
    // As vector is a class & it does not implement addition with operator+(), so this syntax is invalid
    //   "Invalid operands to binary expression ('std::vector<int>' and 'unsigned int')"
    // REQUIRE(*(vector + i) == i + 1)

    // A C array behaves as a pointer in this regard, which is why the syntax is valid there; with vector
    // it only works if you use the provided method to get its data pointer (vector is contiguous by standard, and
    // although operator[]() behavior is implementation defined, this is also how it's implemented everywhere)
    REQUIRE(*(vector.data() + i) == i + 1);

    // This works because vector implements operator[]() by standard
    REQUIRE(vector[i] == i + 1);
    // This works because vector implements at(index) by standard
    REQUIRE(vector.at(i) == i + 1);
  }

  // ----- this section covers how the memory layout differs between C arrays and vectors

  // sizeof is a C builtin for reporting the size individual objects occupy in memory
  // sizeof(array) directly corresponds to item count; it's the size of each item (in bytes) * count
  // sizeof(vector) describes the size of the container object, and none its sub-allocations
  auto sizeof_vec = std::vector{1, 2, 3, 4};
  auto initial_sizeof_vec = sizeof(sizeof_vec);

  auto append_vec = std::vector{5, 6, 7, 8};
  sizeof_vec.reserve(sizeof_vec.size() + append_vec.size());
  sizeof_vec.insert(sizeof_vec.end(), append_vec.begin(), append_vec.end());
  sizeof_vec.shrink_to_fit(); // just in case this had anything to do with it (no)

  REQUIRE(sizeof_vec.size() == 8);                   // the vector indeed has 8 items
  REQUIRE(initial_sizeof_vec == sizeof(sizeof_vec)); // sizeof reads the same as it did when it had 4 items

  // sizeof on the array returns count * individual size; sizeof on an item returns individual size
  // if the array is empty this indexing is an instant crash
  auto c_array_items = sizeof(array) / sizeof(uint32_t);
  REQUIRE(sizeof(array) == c_array_items * sizeof(array[0]));
  REQUIRE(c_array_items == 16);
}
// NOLINTEND
