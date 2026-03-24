#include "Application.h"
using enum EApplicationError;
using Error = ApplicationError;

// Cannot return from calling scope with a function, must use macro
// NOLINTNEXTLINE(*-macro-usage)
#define PROPAGATE_ERROR(CONTEXT, OPERATION)                                                        \
  if (auto err = OPERATION; err != std::nullopt) {                                                 \
    return {CONTEXT, err.context()};                                                               \
  }

Error Application::onInit() {
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING, "Game Game");
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_VERSION_STRING, "1.0");
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_IDENTIFIER_STRING, "com.simewan.adventure");
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING, "sim & ewan");
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_COPYRIGHT_STRING, "sim & ewan 2026");
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING, "");
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_TYPE_STRING, "game");

  /// Order is very important: input is initialized with a reference to window
  PROPAGATE_ERROR(INIT, m_audio.init());
  PROPAGATE_ERROR(INIT, m_graphics.init());
  PROPAGATE_ERROR(INIT, m_window.init());
  PROPAGATE_ERROR(INIT, m_input.init());

  PROPAGATE_ERROR(INIT, m_graphics.attachWindow(m_window));

  m_window.show().mapError(logPassiveError);

  m_isTicking = true;

  return {};
}

Error Application::onDestroy() {
  /// Order is very important: reverse of initialization order
  PROPAGATE_ERROR(DESTROY, m_input.destroy());
  PROPAGATE_ERROR(DESTROY, m_window.destroy());
  PROPAGATE_ERROR(DESTROY, m_graphics.destroy());
  PROPAGATE_ERROR(DESTROY, m_audio.destroy());
  return {};
}

Error Application::onUpdate() {
  SDL_GPUTexture *tex{};
  m_graphics.beginFrame(m_window).mapError(logPassiveError);

  m_audio.update().mapError(logPassiveError);
  m_graphics.update().mapError(logPassiveError);
  m_window.update().mapError(logPassiveError);
  m_input.update().mapError(logPassiveError);

  double lastFrameTime{};
  m_graphics.endFrame(lastFrameTime).mapError(logPassiveError);
#if (APPLICATION_DEBUGGING == true)
  std::println(stdout, "frame time: {}", lastFrameTime);
#endif
  return {};
}

Error Application::onEvent(Event &t_evt) {

  //    window.setTicking(false).mapError(logPassiveError);
  //    window.event(evt.window).mapError(logPassiveError);

  // TODO: process in parallel with corral
  for (const auto &sub : eventSubscriptions) {
    if (t_evt.second.type >= sub.second.first && t_evt.second.type <= sub.second.last) {
      sub.first(t_evt).mapError(logPassiveError);
    }
  }

  // Give subscribers a chance to handle application events (e.g. save before quit)
  // Since above runs in parallel, consumption of the event won't affect whether it's called for
  // remaining subsystems (all subscribers still act on it regardless of which gets the first
  // chance)
  if (!t_evt.first) {
    switch (t_evt.second.type) {
    case SDL_EVENT_QUIT:
      std::println("supposedly quitting");
      PASS_ERROR(setTicking(false))
      break;

    case SDL_EVENT_SYSTEM_THEME_CHANGED:
      std::println("wdym SDL_EVENT_SYSTEM_THEME_CHANGED");
      break;
    case SDL_EVENT_LOCALE_CHANGED:
      std::println("wdym SDL_EVENT_LOCALE_CHANGED");
      break;
    case SDL_EVENT_KEYMAP_CHANGED:
      std::println("wdym SDL_EVENT_KEYMAP_CHANGED");
      break;
    case SDL_EVENT_KEYBOARD_ADDED:
      std::println("wdym SDL_EVENT_KEYBOARD_ADDED");
      break;
    case SDL_EVENT_KEYBOARD_REMOVED:
      std::println("wdym SDL_EVENT_KEYBOARD_REMOVED");
      break;
    case SDL_EVENT_CLIPBOARD_UPDATE:
      std::println("wdym SDL_EVENT_CLIPBOARD_UPDATE");
      break;
    case SDL_EVENT_DROP_FILE:
      std::println("wdym SDL_EVENT_DROP_FILE");
      break;
    case SDL_EVENT_DROP_TEXT:
      std::println("wdym SDL_EVENT_DROP_TEXT");
      break;
    case SDL_EVENT_DROP_BEGIN:
      std::println("wdym SDL_EVENT_DROP_BEGIN");
      break;
    case SDL_EVENT_DROP_COMPLETE:
      std::println("wdym SDL_EVENT_DROP_COMPLETE");
      break;
    case SDL_EVENT_DROP_POSITION:
      std::println("wdym SDL_EVENT_DROP_POSITION");
      break;
      std::println("unimplemented feature requested");
      break;
    default:
#if (APPLICATION_DEBUGGING == true)
      std::println("event dropped {}", t_evt.second.type);
#endif
      return {};
    }
  }
  return {};
}

// ----------

TEST_CASE("Application class can be contained in single page on host system", "") {
  REQUIRE(sizeof(Application) <= Application::getHostPageSize());
}
