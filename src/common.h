// This file & its includes will be automatically sourced by CMake
#pragma once

// IWYU pragma: begin_keep
#include <SDL3/SDL.h>

#include <memory>
#include <optional>
#include <print>
#include <string>
// IWYU pragma: end_keep

/// This is a way we can propagate errors all the way up & keep context
/// T is an enum class type describing possible error subcategories, which get matched at the print site to give an
/// error like "error while initializing audio: error" or "error while stopping graphics: error". Functions which are
/// expected to return a value and potentially fail should return this and take output as an argument
template <typename T> using ErrorBase = std::optional<std::tuple<T, const char *>>;
