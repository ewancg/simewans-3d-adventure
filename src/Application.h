#pragma once
#include "Audio.h"
#include "Graphics.h"
#include "Input.h"
#include "Subsystem.h"
#include "Window.h"

#include <unistd.h>

#ifndef APPLICATION_DEBUGGING
#define APPLICATION_DEBUGGING false
#endif

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing the application") E(DESTROY, "destroying the application")
DEFINE_DERIVED_ERROR_TYPES(Application, Subsystem, ERROR_ENTRIES);
#undef ERROR_ENTRIES

class Application : public Subsystem<ApplicationError> {
  SUBSYSTEM(Application);

public:
  // NOLINTBEGIN (*-non-private-member-variables-in-classes)
  // Object parameters must be populated at the callsite before initialization
  Audio m_audio{};
  Graphics m_graphics{};
  Window m_window{};
  Input m_input = Input(m_window);
  // NOLINTEND

  static uint64_t getHostPageSize() { return uint64_t(sysconf(_SC_PAGESIZE)); };

  DEFINE_PROPERTY(bool, m_isTicking, isTicking, setTicking, false);
};
