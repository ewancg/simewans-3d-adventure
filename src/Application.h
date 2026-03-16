#pragma once
#include "Audio.h"
#include "Graphics.h"
#include "Input.h"
#include "Subsystem.h"
#include "Window.h"

#ifndef APPLICATION_DEBUGGING
#define APPLICATION_DEBUGGING false
#endif

#define ERROR_ENTRIES(E) E(INIT, "") E(DESTROY, "")

DEFINE_DERIVED_ERROR_TYPES(Application, ESubsystemError::END, SubsystemError, ESubsystemError,
                           ERROR_ENTRIES);
#undef ERROR_ENTRIES

class Application : public Subsystem<ApplicationError> {
public:
  using Error = ApplicationError;
  Error onInit();
  Error onDestroy();
  Error onUpdate();

private:
  Audio m_audio{};
  Graphics m_graphics{};
  Window m_window{};
  Input m_input = Input(m_window);

  DEFINE_PROPERTY(bool, m_isTicking, isTicking, setTicking, false);
};
