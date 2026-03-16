// This file & its includes will be automatically sourced by CMake
#pragma once

// IWYU pragma: begin_keep
/// External
#include <SDL3/SDL.h>
#include <format>
// #include <miniaudio/miniaudio.h>
/// STL
#include <functional>
#include <memory>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

// IWYU pragma: end_keep
/// ----- Global project defines -----

constexpr auto NS_PER_SEC = 1000000000.0;
constexpr auto NSPF_60FPS = (NS_PER_SEC / 60.);
#include "macros.h"

/// ----- Global types -----

/// DEFINE_ERROR_TYPES(BaseName, BaseType, ERRORS_LIST): a macro for creating specialized error
/// types for ease of error delivery. It creates 2 types: a type-safe enum deriving from the
/// integer or enum type passed as BaseType under the name E<BaseName>Error, and a "context"
/// type <BaseName>Error which is an optional wrapping a tuple object which can be easily used
/// and unfolded anywhere. E<BaseName>Error describes all possible context-specific errors & can
/// inherit from an existing set of definitions (e.g. subsystem errors for opaque operations).
/// <BaseName>Error is returned from anything with foreseeable failure, and it can be checked
/// like if(error), which is very powerful when combined with `if(auto x = ...; x)`. It also
/// provides
/// `.mapError(F(e))` to take a function to act upon the error, currently there is no way to exit
/// the local scope after this operation so you will end up having to check it unless you're
/// just printing or crudely aborting. It can be expanded to its base parts (E<BaseName>Error,
/// message) when it needs to be printed or acted upon: using `auto [error_type, msg] = *error`,
/// you can match on the source of the error to adapt to it, get its string to print the context
/// + specific error via `error_type.context()`.
///
/// As mentioned, anything under the subsystem which can encounter an error during its operation
/// should return an Error and if it's expected to output something, it should take reference
/// (&) to a pre-allocated object to mutate. These errors are generic and can either be bubbled
/// up or printed & acted upon anywhere they are encountered.

/// It contains a string field so we can propagate errors all the way up while keeping context &
/// not requiring the callsite to know what happened. T is an enum class type describing
/// possible error subcategories, which get matched at the print site to give an error like
/// "error while initializing audio: error" or "error while stopping graphics: error". Functions
/// which are expected to return a value and potentially fail should return this and take output
/// as an argument

template <typename T, typename Base = std::optional<std::tuple<T, const std::string_view>>>
class ErrorBase : public Base {
public:
  using Base::Base;
  using Base::operator=;
  using Base::operator*;
  using Base::operator bool;

  template <typename uint = std::underlying_type<T>>
  constexpr ErrorBase(const uint &t_val, const std::string_view msg)
      : Base(std::in_place, std::move(static_cast<T>(t_val)), msg) {}

  constexpr ErrorBase(T &&t_val, const std::string_view msg)
      : Base(std::in_place, std::move(t_val), msg) {}
  constexpr ErrorBase(const T &t_val, const std::string_view msg)
      : Base(std::in_place, t_val, msg) {}

  void mapError(this auto const &t_self, auto &&t_fun) {
    if (t_self) {
      std::forward<decltype(t_fun)>(t_fun)(t_self);
    }
  }
  constexpr std::string_view string(this auto const &t_self) {
    if (!t_self) {
      return "";
    }
    auto [type, str] = *t_self;
    auto typeStr = t_self.context();
    static thread_local std::string buffer{};
    buffer = std::format("{}: {}", typeStr, str);
    return buffer;
  }
};

const static auto logPassiveError = []<typename E>(E t_err) {
  std::println(stderr, "Error encountered {}", t_err.string());
};
