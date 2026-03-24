#pragma once

#include "../Subsystem.h"

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing the application")                                                          \
  E(DESTROY, "destroying the application")                                                         \
  E(CONFIG_INIT, "getting the configuration path")                                                 \
  E(CONFIG_READ, "loading configurations from disk")                                               \
  E(CONFIG_WRITE, "writing configurations to disk")                                                \
  E(CONFIG_SERIALIZE, "converting an in-memory data type to string")                               \
  E(CONFIG_DESERIALIZE, "converting a string to an in-memory data type")
DEFINE_DERIVED_ERROR_TYPES(Application, Subsystem, ERROR_ENTRIES);
//_DEFINE_ERROR_ENUM_TYPE(Application, _ERROR_ENUM_NAME(Subsystem)::END, ENTRIES);
//_DEFINE_DERIVED_ERROR_CONTEXT_TYPE(Application, _ERROR_ENUM_NAME(Subsystem)::END,
//                                   _ERROR_CONTEXT_NAME(Subsystem), _ERROR_ENUM_NAME(Subsystem),
//                                   ENTRIES)
#undef ERROR_ENTRIES
