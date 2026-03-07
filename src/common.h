// This file & its includes will be automatically sourced by CMake
#pragma once

// IWYU pragma: begin_keep
#include <SDL3/SDL.h>

#include <memory>
#include <optional>
#include <print>
#include <string>
#include <tuple>
#include <utility>
// IWYU pragma: end_keep

/// This is a way we can propagate errors all the way up & keep context
/// T is an enum class type describing possible error subcategories, which get matched at the print site to give an
/// error like "error while initializing audio: error" or "error while stopping graphics: error". Functions which are
/// expected to return a value and potentially fail should return this and take output as an argument
// template <typename T> using ErrorBase = std::optional<std::tuple<T, const char *>>;

template <typename T, typename Base = std::optional<std::tuple<T, const char *>>> class ErrorBase : public Base {
public:
  using Base::Base;
  using Base::operator=;
  using Base::operator*;
  using Base::operator bool;

  constexpr ErrorBase(T &&val, const char *msg) : Base(std::in_place, std::move(val), msg) {}
  constexpr ErrorBase(const T &val, const char *msg) : Base(std::in_place, val, msg) {}

  void map_err(this auto const &self, auto &&fun) {
    if (self) {
      std::forward<decltype(fun)>(fun)(self);
    }
  }
};

// clang-format off
#define ERROR_CONTEXT_TYPE(BODY)                               \
  struct Error : public ErrorBase<EError> {                    \
    using ErrorBase::ErrorBase;                                \
    using ErrorBase::operator=;                                \
    [[nodiscard]] constexpr std::string_view context() const { \
      using enum EError;                                       \
      switch (std::get<0>(**this)) {                           \
        BODY                                                   \
      }                                                        \
    }                                                          \
  };
// clang-format on

struct Graphics;
struct Input;
struct Window;
