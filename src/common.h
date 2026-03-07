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

/// ----- Trivial #defines -----

#define TICK_DIVISOR 1000000000.0
#define TICK_INTERVAL_60FPS 16666667 // 16666667 is the nanoseconds per frame at 60 fps

/// ----- Common types and macros -----

/// This is a way we can propagate errors all the way up & keep context
/// T is an enum class type describing possible error subcategories, which get matched at the print
/// site to give an error like "error while initializing audio: error" or "error while stopping
/// graphics: error". Functions which are expected to return a value and potentially fail should
/// return this and take output as an argument

template <typename T, typename Base = std::optional<std::tuple<T, const char *>>>
class ErrorBase : public Base {
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

// uses "deducing this" to call the init callbacks on the real type for the current instance
template <typename Error> class Subsystem {
  bool m_initialized{};

public:
  /// For the child class to implement
  Error onInit();
  Error onDestroy();

  /// Populates a default-constructed instance (can error).
  Error init(this auto &self) {
    if (auto error = self.onInit(); error) {
      return error;
    }
    self.m_initialized = true;
    return {};
  };

  /// Destroys an instance (can error).
  Error destroy(this auto &self) {
    self.m_initialized = false;
    if (auto error = self.onDestroy(); error) {
      return error;
    }
    return {};
  }

  bool initialized() { return m_initialized; };
};

#define ERROR_CONTEXT_TYPE_NAMED(ENUM_NAME, CONTEXT_NAME, BODY)                                    \
  struct CONTEXT_NAME : public ErrorBase<ENUM_NAME> {                                              \
    using ErrorBase::ErrorBase;                                                                    \
    using ErrorBase::operator=;                                                                    \
    [[nodiscard]] constexpr std::string_view context() const {                                     \
      using enum ENUM_NAME;                                                                        \
      switch (std::get<0>(**this)) { BODY }                                                        \
    }                                                                                              \
  }

#define ERROR_ENUM_ENTRY(name, str) name,
#define ERROR_CONTEXT_ENTRY(name, str)                                                             \
  case name:                                                                                       \
    return str;

#define DEFINE_ERROR_TYPES(NAME, ENTRIES)                                                          \
  enum class E##NAME##Error : uint8_t{ENTRIES(ERROR_ENUM_ENTRY)};                                  \
  ERROR_CONTEXT_TYPE_NAMED(E##NAME##Error, NAME##Error, {ENTRIES(ERROR_CONTEXT_ENTRY)})

#define DEFINE_PROPERTY_COMMON(TYPE, NAME)                                                         \
private:                                                                                           \
  TYPE NAME;

#define DEFINE_PROPERTY(TYPE, NAME, GETTER, SETTER)                                                \
  DEFINE_PROPERTY_COMMON(TYPE, NAME)                                                               \
public:                                                                                            \
  [[nodiscard]] TYPE GETTER() const { return NAME; }                                               \
  void SETTER(TYPE val) { (NAME) = val; }                                                          \
                                                                                                   \
private:

#define DEFINE_REF_PROPERTY(TYPE, NAME, GETTER, SETTER)                                            \
  DEFINE_PROPERTY_COMMON(TYPE, NAME)                                                               \
public:                                                                                            \
  [[nodiscard]] const TYPE &GETTER() const { return NAME; }                                        \
  void SETTER(const TYPE &val) { (NAME) = val; }                                                   \
                                                                                                   \
private:
