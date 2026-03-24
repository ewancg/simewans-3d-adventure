#pragma once
#include "Application.h"
#include <any>
#include <filesystem>
#include <glaze/toml.hpp>

struct Color {
  uint8_t red{}, green{}, blue{}, alpha{};
};

template <typename T> struct SerializablePOD : std::pair<const char *, T> {
  inline ApplicationError toString(std::string &t_output);
  inline ApplicationError fromString();
  inline ApplicationError reload(std::string_view t_filePath);
};

struct MainData {
  Color null_brush{};
};
struct GraphicsData {
  bool vsync{};
  std::optional<float> dpi_override;
  std::optional<uint16_t> fps_cap;
};

class Config {
  std::filesystem::path m_gamePath, m_configPath;
  Application &m_app;
  SerializablePOD<MainData> m_main;
  SerializablePOD<GraphicsData> m_graphics;

public:
  NO_COPY_MOVE_OR_ASSIGN(Config, "", "")
  enum class ESystemConfigs : uint8_t { MAIN, GRAPHICS, SC_LENGTH };
  enum class EConfigType : uint8_t { SYSTEM_LOCAL = 0, GAME_LOCAL };

  std::any operator[](ESystemConfigs t_type);

  explicit Config(Application &t_app);
  ~Config() = default;

  [[nodiscard]] std::string getPath(EConfigType t_type, std::string t_inPath = "") const;

private:
  std::array<SerializablePOD<std::any>, uint8_t(ESystemConfigs::SC_LENGTH)> m_systemConfigs{
      m_main, m_graphics};
};
