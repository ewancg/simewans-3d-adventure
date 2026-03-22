#pragma once

#include <functional>
#include <type_traits>
template <typename T> constexpr std::string_view noContext(T &) { return ""; }

#define ERROR_ENTRIES(E)                                                                           \
  E(ACCESS_WITHOUT_INIT, "executing an operation before initialization")                           \
  E(DELETE_AFTER_FRAME, "executing a frame-end deferred deletion request")                         \
//  DEFINE_BASE_ERROR_TYPES(Subsystem, ERROR_ENTRIES)
// these are supposed to be the expanded result of the above macro call, no idea why it breaks
_DEFINE_ERROR_ENUM_TYPE(Subsystem, 0, ERROR_ENTRIES)
_DEFINE_BASE_ERROR_CONTEXT_TYPE(Subsystem, ERROR_ENTRIES)
#undef ERROR_ENTRIES

template <typename Error>
concept IsSubsystemError = bool(std::is_base_of<SubsystemError, Error>());

template <IsSubsystemError Error> class Subsystem;

template <typename T, typename Error>
concept IsSubsystem = bool(std::is_base_of<Subsystem<Error>, T>());

template <IsSubsystemError Error = SubsystemError> class Subsystem {
private:
  using enum ESubsystemError;
  bool m_isInitialized{};

  std::vector<std::reference_wrapper<const std::function<Error()>>> m_pending_deletion_callbacks;

protected:
  /// For the child class to implement
  Error onInit();
  Error onDestroy();
  Error onUpdate();

public:
  // uses "deducing this" to call the init callbacks on the real type for the current instance

  /// Populates a default-constructed instance (can error).
  Error init(this auto &t_self) {
    PASS_ERROR(t_self.onInit())
    t_self.m_isInitialized = true;
    return {};
  }
  bool isInitialized() { return m_isInitialized; };

  /// Destroys an instance (can error).
  Error destroy(this auto &t_self) {
    t_self.m_isInitialized = false;
    return t_self.onDestroy();
  }

  /// Ticks an instance
  Error update(this auto &t_self) {
    PASS_ERROR(ensureInitialized<Error>(t_self, "subsystem update called"))
    return t_self.onUpdate();
    // Execute deferred deletions
    auto batch = std::move(t_self.m_pending_deletion_callbacks);
    for (auto &deleter : batch) {
      auto err = deleter();
      if (err) {
        return {DELETE_AFTER_FRAME, err.string()};
      }
    }
  }

  /// Protects against double deletes across varied contexts + allows us to assume every stateful
  /// object remains valid throughout the frame lifetime, even if something unrelated in the frame
  /// caused it to invalidate for future use. Saves us from having to delete it next frame by
  /// storing its validity state or checking the specific conditions under which it would be
  /// erroneous.
  template <typename T, IsSubsystemError E>
  void deleteAfterFrame(const std::function<E()> &t_deleterFn) {
    static_assert(
        std::has_virtual_destructor_v<T> || std::is_final_v<T>,
        "A Subsystem cannot manage the lifetime of an item without a virtual destructor.");
    m_pending_deletion_callbacks.push_back(std::reference_wrapper(t_deleterFn));
  }
  template <typename T> void deleteAfterFrame(std::unique_ptr<T> t_object) {
    deleteAfterFrame(t_object.release());
  }

  template <IsSubsystemError ForeignError, IsSubsystem<ForeignError> T>
  static ForeignError ensureInitialized(T &t_self, std::string_view t_msg) noexcept {
    if (!t_self.m_isInitialized) {
      return {ESubsystemError::ACCESS_WITHOUT_INIT, t_msg};
    }
    return {};
  }
};
