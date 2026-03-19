// ----- Private -----

// Error construction macros

#define _ERROR_ENUM_NAME(NAME) E##NAME##Error
#define _ERROR_ENUM_ENTRY(CONSTANT, MSG) CONSTANT,
#define _DEFINE_ERROR_ENUM_TYPE(NAME, START_INDEX, ENTRIES)                                        \
  enum class _ERROR_ENUM_NAME(NAME) : uint8_t {                                                    \
    START = uint8_t(START_INDEX),                                                                  \
    ENTRIES(_ERROR_ENUM_ENTRY) UNKNOWN,                                                            \
    END                                                                                            \
  };

#define _ERROR_CONTEXT_NAME(NAME) NAME##Error
#define _ERROR_CONTEXT_CASE(CONSTANT, MSG)                                                         \
  case Enum::CONSTANT:                                                                             \
    return MSG;

//
// #define _DEFINE_ERROR_CONTEXT_TYPE(NAME, START_INDEX, PARENT_TYPE, BASE_ENUM_TYPE, IS_DERIVED, \
//                                    ENTRIES)
// struct _ERROR_CONTEXT_NAME(NAME) : public PARENT_TYPE {
//   using Enum = _ERROR_ENUM_NAME(NAME);
//   using PARENT_TYPE::PARENT_TYPE;
//   using PARENT_TYPE::operator=;
//   [[nodiscard]] constexpr std::string_view context(this auto &self) {
//     if (!self) {
//       return "";
//     }
//     const auto value = std::to_underlying(std::get<0>(*self));
//     const auto &msg = std::get<1>(*self);
// #if IS_DERIVED
//     if (value < uint8_t(START_INDEX)) {
//       PARENT_TYPE base_err{static_cast<BASE_ENUM_TYPE>(value), msg};
//       return base_err.context();
//     }
// #endif
//     switch (static_cast<Enum>(value)) {
//       ENTRIES(_ERROR_CONTEXT_CASE)
//     case Enum::UNKNOWN:
//       [[fallthrough]];
//     default:
//       return "unknown " #NAME " error";
//     }
//   }
// };
// Error construction macros
// #define DEFINE_BASE_ERROR_TYPES(NAME, ENTRIES)
// _DEFINE_ERROR_ENUM_TYPE(NAME, 0, ENTRIES)
// _DEFINE_ERROR_CONTEXT_TYPE(NAME, 0, ErrorBase<_ERROR_ENUM_NAME(NAME)>, _ERROR_ENUM_NAME(NAME), 0,
//                            ENTRIES)
// #define DEFINE_DERIVED_ERROR_TYPES(NAME, START, BASE_ERROR_TYPE, BASE_ENUM_TYPE, ENTRIES)
// _DEFINE_ERROR_ENUM_TYPE(NAME, START, ENTRIES)
// _DEFINE_ERROR_CONTEXT_TYPE(NAME, START, BASE_ERROR_TYPE, BASE_ENUM_TYPE, 1, ENTRIES)

//
#define _DEFINE_BASE_ERROR_CONTEXT_TYPE(NAME, ENTRIES)                                             \
  struct _ERROR_CONTEXT_NAME(NAME) : public ErrorBase<_ERROR_ENUM_NAME(NAME)> {                    \
    using Enum = _ERROR_ENUM_NAME(NAME);                                                           \
    using ErrorBase::ErrorBase;                                                                    \
    using ErrorBase::operator=;                                                                    \
    [[nodiscard]] constexpr std::string_view context(this auto &self) {                            \
      if (!self) {                                                                                 \
        return "";                                                                                 \
      }                                                                                            \
      const auto value = std::to_underlying(std::get<0>(*self));                                   \
      const auto &msg = std::get<1>(*self);                                                        \
      switch (static_cast<Enum>(value)) {                                                          \
        ENTRIES(_ERROR_CONTEXT_CASE)                                                               \
      case Enum::UNKNOWN:                                                                          \
        return "unknown " #NAME " error";                                                          \
      default:                                                                                     \
        return "unknown " #NAME " error";                                                          \
      }                                                                                            \
    }                                                                                              \
  };

#define _DEFINE_DERIVED_ERROR_CONTEXT_TYPE(NAME, START_INDEX, BASE_ERROR_TYPE, BASE_ENUM_TYPE,     \
                                           ENTRIES)                                                \
  struct _ERROR_CONTEXT_NAME(NAME) : public BASE_ERROR_TYPE {                                      \
    using Enum = _ERROR_ENUM_NAME(NAME);                                                           \
    using BASE_ERROR_TYPE::BASE_ERROR_TYPE;                                                        \
    using BASE_ERROR_TYPE::operator=;                                                              \
    [[nodiscard]] constexpr std::string_view context(this auto &self) {                            \
      if (!self) {                                                                                 \
        return "";                                                                                 \
      }                                                                                            \
      const auto value = std::to_underlying(std::get<0>(*self));                                   \
      const auto &msg = std::get<1>(*self);                                                        \
      if (value < uint8_t(START_INDEX)) {                                                          \
        BASE_ERROR_TYPE base_err{static_cast<BASE_ENUM_TYPE>(value), msg};                         \
        return base_err.context();                                                                 \
      }                                                                                            \
      switch (static_cast<Enum>(value)) {                                                          \
        ENTRIES(_ERROR_CONTEXT_CASE)                                                               \
      case Enum::UNKNOWN:                                                                          \
        return "unknown " #NAME " error";                                                          \
      default:                                                                                     \
        return "unknown " #NAME " error";                                                          \
      }                                                                                            \
    }                                                                                              \
  };

// Property definition items
#define _DEFINE_PROPERTY_COMMON(TYPE, NAME, DEFAULT)                                               \
private:                                                                                           \
  TYPE NAME = DEFAULT;

#define _DEFINE_GETTER(TYPE, NAME, GETTER, OPERATION)                                              \
  Error GETTER(this auto &t_self, TYPE &t_out) {                                                   \
    if (Error err = Subsystem::ensureInitialized<decltype(t_self), Error>(                         \
            t_self, "property " #NAME " read");                                                    \
        err) {                                                                                     \
      return err;                                                                                  \
    }                                                                                              \
    t_out = OPERATION(t_self.NAME);                                                                \
    return {};                                                                                     \
  }
// const setter input = forced copy, no need
#define _DEFINE_SETTER(TYPE, NAME, SETTER, OPERATION)                                              \
  Error SETTER(this auto &t_self, TYPE t_val) {                                                    \
    if (Error err = Subsystem::ensureInitialized<decltype(t_self), Error>(                         \
            t_self, "property " #NAME " set");                                                     \
        err) {                                                                                     \
      return err;                                                                                  \
    }                                                                                              \
    t_self.NAME = OPERATION(t_val);                                                                \
    return {};                                                                                     \
  }

/// ----- Public -----

// Error construction macros
#define DEFINE_BASE_ERROR_TYPES(NAME, ENTRIES)                                                     \
  _DEFINE_ERROR_ENUM_TYPE(NAME, 0, ENTRIES)                                                        \
  _DEFINE_BASE_ERROR_CONTEXT_TYPE(NAME, ENTRIES)

#define DEFINE_DERIVED_ERROR_TYPES(NAME, BASE, ENTRIES)                                            \
  _DEFINE_ERROR_ENUM_TYPE(NAME, _ERROR_ENUM_NAME(BASE)::END, ENTRIES)                              \
  _DEFINE_DERIVED_ERROR_CONTEXT_TYPE(NAME, _ERROR_ENUM_NAME(BASE)::END, _ERROR_CONTEXT_NAME(BASE), \
                                     _ERROR_ENUM_NAME(BASE), ENTRIES)

// Property definition macros
#define DEFINE_PROPERTY(TYPE, NAME, GETTER, SETTER, DEFAULT)                                       \
  /* NOLINTBEGIN(*-magic-numbers) */                                                               \
public:                                                                                            \
  _DEFINE_GETTER(TYPE, NAME, GETTER, )                                                             \
  _DEFINE_SETTER(TYPE, NAME, SETTER, )                                                             \
  _DEFINE_PROPERTY_COMMON(TYPE, NAME, DEFAULT)
/* NOLINTEND(*-magic-numbers) */

#define WEAK_REF(TYPE) std::optional<std::reference_wrapper<TYPE>>

#define DEFINE_REF_PROPERTY(TYPE, NAME, GETTER, SETTER, DEFAULT)                                   \
public:                                                                                            \
  _DEFINE_GETTER(TYPE, NAME, GETTER, std::ref)                                                     \
  _DEFINE_SETTER(TYPE, NAME, SETTER, std::move)                                                    \
  _DEFINE_PROPERTY_COMMON(TYPE, NAME, DEFAULT)

#define SUBSYSTEM(NAME)                                                                            \
public:                                                                                            \
  using Error = _ERROR_CONTEXT_NAME(NAME);                                                         \
  friend class Subsystem;                                                                          \
                                                                                                   \
private:                                                                                           \
  Error onInit();                                                                                  \
  Error onDestroy();                                                                               \
  Error onUpdate();
