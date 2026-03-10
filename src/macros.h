// #define DEFINE_ERROR_ENUM_ENTRY(name, msg) name,
//
// #define DEFINE_ERROR_CONTEXT_CASE(ENUM_NAMEname, msg) \
//   case E##NAME##Error::(name): \
//     return msg;
//
// #define DEFINE_ERROR_TYPES(NAME, BASE_TYPE, START, ERRORS_LIST) \
//   enum class E##NAME##Error : BASE_TYPE{_FIRST = (START), /* forces dynamic starting index */ \
//                                         ERRORS_LIST(DEFINE_ERROR_ENUM_ENTRY) UNKNOWN}; \
//                                                                                                    \
//   class NAME##Error final : public ErrorBase<E##NAME##Error> { \
//   public: \
//     using Enum = E##NAME##Error; \
//     using Base = ErrorBase<Enum>; \
//     using Base::Base; \
//     using Base::operator=; \
//     using Base::operator*; \
//     using Base::operator bool; \
//                                                                                                    \
//     constexpr std::string_view context(this const auto &self) noexcept { \
//       if (!self) \
//         return ""; \
//       auto [err, msg] = *self; \
//       auto code = std::to_underlying(err); \
//                                                                                                    \
//       /* Inherited errors (< START) delegate to SubsystemError::context() */ \
//       if (code < (START)) { \
//         SubsystemError base_err{static_cast<ESubsystemError>(code), msg}; \
//         return base_err.context(); \
//       } \
//                                                                                                    \
//       switch (err) { \
//       case Enum::UNKNOWN: \
//         return "unknown " #NAME " error"; \
//         ERRORS_LIST(DEFINE_ERROR_CONTEXT_CASE) \
//       default: \
//         return "unknown " #NAME " error"; \
//       } \
//     } \
//   };

#define DEFINE_ERROR_ENUM_ENTRY(name, msg) name,
#define DEFINE_ERROR_CONTEXT_CASE(name, msg)                                                       \
  case E##NAME##Error::name:                                                                       \
    return msg;
#define ERROR_CONTEXT_TYPE_NAMED(ENUM_NAME, CONTEXT_NAME, BASE_TYPE, BODY)                         \
  struct CONTEXT_NAME : public ErrorBase<ENUM_NAME> {                                              \
    using ErrorBase::ErrorBase;                                                                    \
    using ErrorBase::operator=;                                                                    \
    [[nodiscard]] constexpr std::string_view context() const {                                     \
      /* NOLINT(bugprone-macro-parentheses) */ using enum ENUM_NAME;                               \
      const auto value = std::get<0>(**this);                                                      \
      if (value < UNKNOWN) {                                                                       \
        return BASE_TYPE::context();                                                               \
      }                                                                                            \
      switch (value) { BODY }                                                                      \
    }                                                                                              \
  }

#define ERROR_ENUM_ENTRY(name, str) name,
#define ERROR_CONTEXT_ENTRY(name, str)                                                             \
  case name:                                                                                       \
    return str;

#define DEFINE_ERROR_TYPES(NAME, START, BASE_TYPE, ENTRIES)                                        \
  enum class E##NAME##Error : BASE_TYPE {                                                          \
    UNKNOWN = (START);                                                                             \
    ENTRIES(ERROR_ENUM_ENTRY)                                                                      \
  };                                                                                               \
  ERROR_CONTEXT_TYPE_NAMED(E##NAME##Error, NAME##Error, BASE_TYPE, {ENTRIES(ERROR_CONTEXT_ENTRY)})

#define DEFINE_PROPERTY_COMMON(TYPE, NAME, DEFAULT)                                                \
private:                                                                                           \
  TYPE NAME = DEFAULT;

// #define DEFINE_PROPERTY(TYPE, NAME, GETTER, SETTER) \
//   DEFINE_PROPERTY_COMMON(TYPE, NAME) \
// public: \
//   [[nodiscard]] TYPE GETTER() const { return NAME; } \
//   void SETTER(TYPE val) { (NAME) = val; } \
//                                                                                                    \
// private:
//
// #define DEFINE_REF_PROPERTY(TYPE, NAME, GETTER, SETTER) \
//   DEFINE_PROPERTY_COMMON(TYPE, NAME) \
// public: \
//   [[nodiscard]] const TYPE &GETTER() const { return NAME; } \
//   void SETTER(const TYPE &val) { (NAME) = val; } \
//                                                                                                    \
// private:

#define ASSERT_PROPERTY_INIT(NAME, ERROR_FUNCTION)                                                 \
  if (!m_initialized) {                                                                            \
    return ERROR_FUNCTION(NAME);                                                                   \
  }

#define DEFINE_GETTER(TYPE, NAME, GETTER, OPERATION)                                               \
  Error GETTER(this auto &self, TYPE out) {                                                        \
    if (auto err = self.ensureInitialized("property " #NAME " read prematurely"); err) {           \
      return err;                                                                                  \
    }                                                                                              \
    out = (OPERATION);                                                                             \
    return {};                                                                                     \
  }
/*  const setter input = forced copy, no need */
#define DEFINE_SETTER(TYPE, NAME, SETTER, OPERATION)                                               \
  Error SETTER(this auto &self, TYPE val) {                                                        \
    if (auto err = self.ensureInitialized("property " #NAME " set prematurely"); err) {            \
      return err;                                                                                  \
    }                                                                                              \
    (NAME) = (OPERATION);                                                                          \
    return {};                                                                                     \
  }

#define DEFINE_PROPERTY(TYPE, NAME, GETTER, SETTER, DEFAULT)                                       \
public:                                                                                            \
  DEFINE_GETTER(WEAK_REF(TYPE), NAME, GETTER, NAME)                                                \
  DEFINE_SETTER(TYPE NAME SETTER val)                                                              \
private:                                                                                           \
  DEFINE_PROPERTY_COMMON(TYPE, NAME, DEFAULT)

#define WEAK_REF std::optional<std::reference_wrapper<TYPE>>

#define DEFINE_REF_PROPERTY(TYPE, NAME, GETTER, SETTER, DEFAULT)                                   \
public:                                                                                            \
  DEFINE_GETTER(WEAK_REF(TYPE), NAME, GETTER, std::cref((NAME).value()))                           \
  DEFINE_SETTER(TYPE, NAME, SETTER, std::move(val))                                                \
private:                                                                                           \
  DEFINE_PROPERTY_COMMON(WEAK_REF(TYPE), NAME, DEFAULT)
