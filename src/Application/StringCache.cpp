
#include "StringCache.h"

using enum EApplicationError;

ApplicationError StringCache::init() {
  auto page_size = Application::getHostPageSize();

  // Gets smallest multiple of page size (contiguous pages for us) that is not smaller than our
  // hardcoded constant (expected usage in-game, which can be adjusted)
  size_t size = ((arenaMinimumSize + page_size - 1) / page_size) * page_size;
  auto  *buf  = std::aligned_alloc(page_size, size);
  m_data      = new (buf) StringIndex();
}

ApplicationError StringCache::destroy() {
  m_data->~StringIndex();
  std::free(m_data);
}
