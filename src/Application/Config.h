#pragma once
#include "ConfigDefinitions.h"
#include "Error.h"
#include <any>
#include <filesystem>
#include <glaze/core/context.hpp>
#include <glaze/toml.hpp>
#include <glaze/toml/read.hpp>

template <typename T> class SerializablePOD {
  /// Our settings for reading & writing TOML config files
  struct SystemLocalConfigOpts : glz::opts {

    uint32_t format = glz::TOML;

    bool null_terminated = GLZ_NULL_TERMINATED; /// Whether the input buffer is null terminated
    bool comments        = false;               /// Support reading in JSONC style comments
    bool error_on_unknown_keys = true;          /// Error when an unknown key is encountered
    bool skip_null_members     = true; /// Skip writing out params in an object if the value is null
    bool prettify              = true; /// Write out prettified
    bool minified = false; /// Require minified input, which results in faster read performance
    bool error_on_missing_keys =
        false; /// Require all non nullable keys to be present in the object; Use skip_null_members
               /// = false to require nullable members
    bool partial_read = false; /// Reads into the deepest structural object and then exits without
                               /// parsing the rest of the input
    // when nixpkgs gets GCC 16
    // bool reflect_enums = true;
  };

  T           m_data;
  std::string m_name;

  using enum EApplicationError;

  ApplicationError glzCtxToError(EApplicationError t_type, const glz::error_ctx &t_ctx) {
    if (!t_ctx) {
      return {};
    }
    return ApplicationError{t_type, std::string(t_ctx.custom_error_message)};
  }

public:
  NO_COPY_MOVE_OR_ASSIGN(SerializablePOD, "", "")
  SerializablePOD() : m_data{} {}
  explicit SerializablePOD(std::string_view t_name) : m_name(t_name), m_data{} {}
  ~SerializablePOD() = default;

  std::string &name() { return m_name; }
  T           &data() { return m_data; }
  T           &operator*() { return m_data; }

  ApplicationError toString(std::string &t_output) {
    std::string buffer;
    return glzCtxToError(CONFIG_SERIALIZE, glz::write_toml(t_output, this->m_data, buffer));
  }
  ApplicationError fromString(std::string &t_input) {
    std::string buffer;
    return glzCtxToError(CONFIG_DESERIALIZE, glz::read_toml(this->m_data, t_input, buffer));
  }
  ApplicationError reload(std::string &t_inFilePath) {
    std::string buffer;
    return glzCtxToError(CONFIG_READ, glz::read_file_toml(m_data, t_inFilePath, buffer));
  }
  ApplicationError save(std::string &t_outFilePath) {
    std::string buffer;
    return glzCtxToError(
        CONFIG_WRITE, glz::write_file_toml<SystemLocalConfigOpts{}>(m_data, t_outFilePath, buffer));
  }
};

class Config {
  APPLICATION_PARENT(Config)
  std::filesystem::path               m_gamePath, m_configPath;
  SerializablePOD<MainConfigData>     m_main;
  SerializablePOD<GraphicsConfigData> m_graphics;

  static Config *configPtr;
  static void    emergencySave() { configPtr->save().mapError(logPassiveError); };

public:
  explicit Config(Application &t_app);
  ApplicationError init();
  ApplicationError destroy();
  ~Config() = default;
  ApplicationError update(); // reload the config for whatever reason
  ApplicationError save();   // save the config for whatever reason

  enum class ESystemConfigs : uint8_t { MAIN, GRAPHICS, SC_LENGTH };
  enum class EConfigType : uint8_t { SYSTEM_LOCAL = 0, GAME_LOCAL };

  template <typename T> T &operator[](ESystemConfigs t_type) {
    return std::any_cast<T>(get(t_type));
  }
  [[nodiscard]] std::any get(ESystemConfigs t_type);

private:
  static constexpr std::string_view configStemPath = "/config/";

  [[nodiscard]] std::string formatConfigPath(std::string t_fileName = "") const;
  template <typename T>
  [[nodiscard]] std::string formatConfigFilePath(SerializablePOD<T> &t_input) const;
  [[nodiscard]] std::string getCanonicalConfigBase() const;
  [[nodiscard]] std::string getPath(EConfigType t_type, std::string t_inPath = "") const;
};
Config *Config::configPtr = nullptr;
