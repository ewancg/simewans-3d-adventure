#pragma once

#include <functional>
#include <type_traits>
template <typename T> constexpr std::string_view noContext(T &) { return ""; }

#define ERROR_ENTRIES(E)                                                                           \
  E(ACCESS_WITHOUT_INIT, "executing a post-initialization operation")                              \
  E(DELETE_AFTER_FRAME, "executing a frame-end deferred deletion request")                         \
//  DEFINE_BASE_ERROR_TYPES(Subsystem, ERROR_ENTRIES)
// these are supposed to be the expanded result of the above macro call, no idea why it breaks
_DEFINE_ERROR_ENUM_TYPE(Subsystem, 0, ERROR_ENTRIES)
_DEFINE_BASE_ERROR_CONTEXT_TYPE(Subsystem, ERROR_ENTRIES)
#undef ERROR_ENTRIES

template <typename Error>
concept IsSubsystemError = bool(std::is_base_of<SubsystemError, Error>());

template <IsSubsystemError Error = SubsystemError> class Subsystem {
private:
  bool m_initialized{};

  std::vector<std::reference_wrapper<const std::function<Error()>>> m_pending_deletion_callbacks;

protected:
  /// For the child class to implement
  Error onInit();
  Error onDestroy();
  Error onUpdate();

public:
  // uses "deducing this" to call the init callbacks on the real type for the current instance

  /// Populates a default-constructed instance (can error).
  Error init(this auto &t_self);

  /// Destroys an instance (can error).
  Error destroy(this auto &t_self);

  /// Ticks an instance
  Error update(this auto &t_self);

  /// Protects against double deletes across varied contexts + allows us to assume every stateful
  /// object remains valid throughout the frame lifetime, even if something unrelated in the frame
  /// caused it to invalidate for future use. Saves us from having to delete it next frame by
  /// storing its validity state or checking the specific conditions under which it would be
  /// erroneous.

  template <typename T> void deleteAfterFrame(std::unique_ptr<T> t_object);
  template <typename T> void deleteAfterFrame(const std::function<void()> &t_deleterFn);

  template <typename T, IsSubsystemError ForeignError>
  static ForeignError ensureInitialized(T &t_self, std::string_view t_msg) noexcept;

  bool initialized();
};
