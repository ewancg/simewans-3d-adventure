#include <print>
#include <stdio.h>
#ifndef UNIT_TESTING

int main(int argc, char *argv[]) { std::println("yeah"); }

#else
#include <catch2/catch_test_macros.hpp>
// TEST_CASE("unit test", "") { REQUIRE(true); }
#endif
