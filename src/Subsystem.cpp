#include "Subsystem.h"
using enum ESubsystemError;
using Error = SubsystemError;
template <IsSubsystemError Error> using ForeignError = Error;

Error Subsystem<>::init(this auto &t_self) {
  if (auto error = t_self.onInit(); error) {
    return error;
  }
  t_self.m_initialized = true;
  return {};
};

Error Subsystem<>::endFrame(this auto &t_self) {
  if (auto err = Subsystem::ensureInitialized(t_self, "endFrame called prematurely"); err) {
    return err;
  }
  // Execute deferred deletions
  auto batch = std::move(t_self.m_pending_deletion_callbacks);
  for (auto &deleter : batch) {
    auto err = deleter();
    if (err) {
      return {DELETE_AFTER_FRAME, err.string()};
    }
  }
  return {};
}

Error Subsystem<>::destroy(this auto &t_self) {
  t_self.m_initialized = false;
  return t_self.onDestroy();
}

Error Subsystem<>::update(this auto &t_self) { return t_self.onUpdate(); };

template <>
template <typename T>
void Subsystem<>::deleteAfterFrame(const std::function<void()> &t_deleterFn) {
  static_assert(std::has_virtual_destructor_v<T> || std::is_final_v<T>,
                "A Subsystem cannot manage the lifetime of an item without a virtual destructor.");
  m_pending_deletion_callbacks.push_back(t_deleterFn);
}

template <> template <typename T> void Subsystem<>::deleteAfterFrame(std::unique_ptr<T> t_object) {
  deleteAfterFrame(
      t_object.release()); // Assumes a T* overload exists; define it similarly if needed.
}

template <> bool Subsystem<>::initialized() { return m_initialized; };

template <>
template <typename T, IsSubsystemError ForeignError>
ForeignError Subsystem<>::ensureInitialized(T &t_self, std::string_view t_msg) noexcept {
  if (!t_self.m_initialized) {
    return {ESubsystemError::ACCESS_WITHOUT_INIT, t_msg};
  }
  return {};
}
