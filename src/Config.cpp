#include "Config.h"
#include "../Application.h"

using enum Config::EConfigType;
using enum EApplicationError;

Config::Config(Application &t_app) : m_app(t_app) {
  const auto &metadata = m_app.getMetadata();
  m_gamePath = SDL_GetBasePath();
  if (auto *str = SDL_GetPrefPath(metadata.author, metadata.name); str == nullptr) {
    logPassiveError({EApplicationError::CONFIG_INIT, SDL_GetError()});
  } else {
    m_configPath.assign(str);
  }

  for (uint8_t i = 0; i < static_cast<uint8_t>(ESystemConfigs::SC_LENGTH); i++) {
    auto &config = m_systemConfigs.at(i);
    config.reload(getPath(SYSTEM_LOCAL, std::format("config/{}", config.first)));
  }
}

[[nodiscard]] std::string Config::getPath(EConfigType t_type, std::string t_inPath) const {
  const static auto combinePaths = [](const std::filesystem::path &t_lhs, std::string &t_rhs) {
    auto ret = std::filesystem::canonical(t_lhs);
    ret += t_rhs;
    return ret;
  };
  switch (t_type) {
  case EConfigType::SYSTEM_LOCAL:
    return combinePaths(m_configPath, t_inPath);
  case EConfigType::GAME_LOCAL:
    return combinePaths(m_gamePath, t_inPath);
  default:
    std::unreachable();
  }
}

std::any Config::operator[](ESystemConfigs t_type) { return m_systemConfigs.at(uint8_t(t_type)); }

template <typename T> ApplicationError SerializablePOD<T>::toString(std::string &t_output) {
  if (auto err = glz::write_toml(this->second); err == std::nullopt) {
    return {CONFIG_SERIALIZE,
            std::format("serialization failed for instance of {}", typeid(T).name())};
  } else {
    t_output = *err;
  }
}
template <typename T> ApplicationError SerializablePOD<T>::fromString(std::string &t_input) {
  this->second = glz::read_toml<T>(t_input);
}
template <typename T> ApplicationError SerializablePOD<T>::reload(std::string_view t_filePath) {}
