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
  /// Order is very important: input is initialized with a reference to window
  PROPAGATE_ERROR(INIT, m_audio.init());
  PROPAGATE_ERROR(INIT, m_graphics.init());
  PROPAGATE_ERROR(INIT, m_window.init());
  PROPAGATE_ERROR(INIT, m_input.init());

  m_window.show().mapError(logPassiveError);

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
  m_graphics.getWindowSwapchainTexture(m_window, tex).mapError(logPassiveError);
  m_graphics.beginFrame(tex).mapError(logPassiveError);

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

// ----------

TEST_CASE("Application class can be contained in single page on host system", "") {
  REQUIRE(sizeof(Application) <= Application::getHostPageSize());
}
