#include "Application.h"
#include "Application/Config.h"
#include <any>
#include <print>

using enum Config::EConfigType;
using enum Config::ESystemConfigs;
using enum EApplicationError;
using Error = ApplicationError;

// Cannot return from calling scope with a function, must use macro
// NOLINTNEXTLINE(*-macro-usage)
#define PROPAGATE_ERROR(CONTEXT, OPERATION)                                                        \
  if (auto err = OPERATION; err != std::nullopt) {                                                 \
    return {CONTEXT, err.context()};                                                               \
  }

Error Application::onInit() {
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING, metadata.name);
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_VERSION_STRING, metadata.version);
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_IDENTIFIER_STRING, metadata.identifier);
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING, metadata.author);
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_COPYRIGHT_STRING, metadata.copyright);
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING, metadata.url);
  SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_TYPE_STRING, metadata.type);

  /// Load the config because it's guaranteed for subsystems at init
  PROPAGATE_ERROR(INIT, config.init());

  /// Order is very important: input is initialized with a reference to window
  PROPAGATE_ERROR(INIT, audio.init());
  PROPAGATE_ERROR(INIT, graphics.init());
  PROPAGATE_ERROR(INIT, window.init());
  PROPAGATE_ERROR(INIT, input.init());

  PROPAGATE_ERROR(INIT, graphics.attachWindow(window));

  window.show().mapError(logPassiveError);

  m_isTicking = true;
  return {};
}

Error Application::onDestroy() {
  if (m_isTicking) {
    logPassiveError(Error{DESTROY, "destroy called while app is supposedly still ticking"});
  }

  window.close().mapError(logPassiveError);

  /// Save the config before it potentially crashes on shutdown & causes a quick_exit
  PROPAGATE_ERROR(DESTROY, config.destroy());

  /// Subsystem destruction order is important: reverse of initialization order
  PROPAGATE_ERROR(DESTROY, input.destroy());
  PROPAGATE_ERROR(DESTROY, window.destroy());
  PROPAGATE_ERROR(DESTROY, graphics.destroy());
  PROPAGATE_ERROR(DESTROY, audio.destroy());
  return {};
}

Error Application::onUpdate() {
  SDL_GPUTexture *tex{};
  graphics.beginFrame(window).mapError(logPassiveError);

  audio.update().mapError(logPassiveError);
  graphics.update().mapError(logPassiveError);
  window.update().mapError(logPassiveError);
  input.update().mapError(logPassiveError);

  double lastFrameTime{};
  graphics.endFrame(lastFrameTime).mapError(logPassiveError);
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

  auto mainConfig     = std::any_cast<MainConfigData>(config.get(MAIN));
  auto graphicsConfig = std::any_cast<GraphicsConfigData>(config.get(GRAPHICS));

  if (!t_evt.first) {
    switch (t_evt.second.type) {
    case SDL_EVENT_QUIT:
      PASS_ERROR(setTicking(false))
      break;

    case SDL_EVENT_KEY_DOWN:
      std::println("{}, {}, {}, {}", mainConfig.null_brush_color.r, mainConfig.null_brush_color.g,
                   mainConfig.null_brush_color.b, mainConfig.null_brush_color.a);
      std::println("{}, {}, {}", graphicsConfig.dpi_override, graphicsConfig.fps_cap,
                   graphicsConfig.vsync);
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
