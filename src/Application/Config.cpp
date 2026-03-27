#include "Config.h"
#include "../Application.h"
#include "Error.h"

using enum Config::EConfigType;
using enum Config::ESystemConfigs;
using enum EApplicationError;
Config *Config::configPtr = nullptr;

Config::Config(Application &t_app) : m_app(t_app), m_main("main"), m_graphics("graphics") {
  if (configPtr != nullptr) {
    logPassiveError(ApplicationError{
        CONFIG_INIT, "Attempted to create a duplicate Application::Config instance"});
  }
  configPtr = this;
};

ApplicationError Config::init() {
  m_gamePath = SDL_GetBasePath();
  auto *str  = SDL_GetPrefPath(Application::metadata.author, Application::metadata.name);
  if (const auto *err = SDL_GetError(); str == nullptr || err != nullptr) {
    auto res = std::string(err);
    std::format_to(&res, "couldn't determine config path ({})", res);
    return {CONFIG_INIT, res};
  }
  m_configPath.assign(str);
  auto baseDir = formatConfigPath();
  if (!std::filesystem::exists(baseDir)) {
    if (!SDL_CreateDirectory(baseDir.c_str())) {
      auto str =
          errorStr(m_app, std::format("couldn't create config directory ({})", SDL_GetError()));
      return {CONFIG_INIT, std::string_view(*str)};
    }
  }
  if (std::at_quick_exit(&Config::emergencySave) != 0) {
    logPassiveError(
        ApplicationError{CONFIG_INIT, "unable to register at_quick_exit hook for saving configs"});
  }
  PASS_ERROR(update())
}

ApplicationError Config::update() {
  auto configPath = formatConfigFilePath(m_main);
  PASS_ERROR(m_main.reload(configPath))
  configPath = formatConfigFilePath(m_graphics);
  PASS_ERROR(m_graphics.reload(configPath))
  return {};
}

ApplicationError Config::destroy(){PASS_ERROR(save())}

ApplicationError Config::save() {
  auto configPath = formatConfigFilePath(m_main);
  PASS_ERROR(m_main.save(configPath));
  configPath = formatConfigFilePath(m_graphics);
  PASS_ERROR(m_graphics.save(configPath));
  return {};
}

std::any Config::get(ESystemConfigs t_type) {
  switch (t_type) {
  case MAIN:
    return std::any{m_main.data()};
  case GRAPHICS:
    return std::any{m_graphics.data()};
  default:
    std::unreachable();
  }
}

std::string Config::formatConfigPath(std::string t_fileName) const {
  return getPath(SYSTEM_LOCAL, std::move(t_fileName));
};
template <typename T> std::string Config::formatConfigFilePath(SerializablePOD<T> &t_input) const {
  return formatConfigPath(t_input.name() + ".toml");
};

std::string Config::getPath(EConfigType t_type, std::string t_inPath) const {
  const static auto combinePaths = [](const std::filesystem::path &t_lhs,
                                      std::string                 &t_rhs) -> std::string {
    if (t_lhs.empty()) {
      return "t_lhs";
    }
    return std::filesystem::canonical(t_lhs).concat(configStemPath).concat(t_rhs).string();
  };
  switch (t_type) {
  case SYSTEM_LOCAL:
    return combinePaths(m_configPath, t_inPath);
  case GAME_LOCAL:
    return combinePaths(m_gamePath, t_inPath);
  default:
    std::unreachable();
  }
}
