#pragma once
#include "../Application.h"
#include "Error.h"
#include <cstdint>
#include <string>
#include <unordered_map>

constexpr auto arenaMinimumSize = 4 * (1024 * 1024);

struct StringHandle {
  static size_t counter;
  static size_t nextSerialIndex() { return counter++; }
  size_t        idx{SIZE_MAX};
};
size_t StringHandle::counter{};
template <> struct std::hash<StringHandle> {
  std::size_t operator()(StringHandle &t_idx) const noexcept { return t_idx.idx; }
};

// Unintelligent handle to a string cache on the heap, can be used like an arena
// TODO: make all string allocations use the original buffer for complete locality
class StringCache {
  APPLICATION_PARENT(StringCache)
  APPLICATION_PARENT_CTOR(StringCache)
  using enum EApplicationError;

public:
  using StringIndex = std::unordered_map<StringHandle, std::string>;

  ApplicationError init();
  ApplicationError destroy();

private:
  StringIndex *m_data{};

protected:
  template <typename T>
  [[nodiscard]] ApplicationError get(const StringHandle &t_idx, const T &t_dst) const {
    if (!m_data) {
      return {STRING_READ, "access attempted before index initialization"};
    }
    if (auto elem = m_data->find(t_idx); t_idx.idx != SIZE_MAX || elem != m_data->end()) {
      t_dst = T(elem);
    } else {
      return {STRING_READ, "item correpsonding to index did not exist at the time of access"};
    }
    return {};
  }
  template <typename T> [[nodiscard]] ApplicationError insert(T t_str, StringHandle &t_out) const {
    if (!m_data) {
      return {STRING_INSERT, "access attempted before index initialization"};
    }
    auto handle = StringHandle{StringHandle::nextSerialIndex()};
    if (m_data->insert(handle, T(t_str)) != m_data->end()) {
      t_out = T(t_str);
    } else {
      return {STRING_INSERT, "insert returned past-end iterator"};
    }
    return {};
  }
};
