#include <frozen/unordered_map.h>
enum class EGPUShaderID : uint8_t { VERTEX, FRAGMENT, PIXEL, LENGTH };
// NOLINTBEGIN(*-avoid-c-arrays, *-extensions)
namespace shader_definitions {
constexpr unsigned char vertex[] = {
#embed "../../out/shaders/vertex-bytecode"
};

constexpr unsigned char fragment[] = {
#embed "../../out/shaders/fragment-bytecode"
};

constexpr unsigned char pixel[] = {
#embed "../../out/shaders/pixel-bytecode"
};
static constexpr std::array<std::pair<EGPUShaderID, std::span<const unsigned char>>,
                            uint8_t(EGPUShaderID::LENGTH)>
    index{
        {{EGPUShaderID::VERTEX, {static_cast<const unsigned char *>(vertex), sizeof(vertex)}},
         {EGPUShaderID::FRAGMENT, {static_cast<const unsigned char *>(fragment), sizeof(fragment)}},
         {EGPUShaderID::PIXEL, {static_cast<const unsigned char *>(pixel), sizeof(pixel)}}}};

} // namespace shader_definitions
// NOLINTEND(*-avoid-c-arrays, *-extensions)
//
static constexpr auto shaders = frozen::make_unordered_map(shader_definitions::index);
class GPUShader {
  SDL_GPUDevice *m_device{};
  SDL_GPUShader *m_shader{};
  SDL_GPUShaderStage m_stage{};
  SDL_GPUShaderFormat m_format{};
  EGPUShaderID m_shaderID{EGPUShaderID::LENGTH};
  // Uint8 *m_bytecode;
public:
  friend class Graphics;
  NO_COPY_MOVE_OR_ASSIGN(GPUShader, "cannot copy shaders, they are unique handles",
                         "cannot move shaders")

  GPUShader(SDL_GPUDevice *t_device, SDL_GPUShaderStage t_stage, SDL_GPUShaderFormat t_format,
            EGPUShaderID t_shaderID)
      : m_device(t_device), m_stage(t_stage), m_format(t_format), m_shaderID(t_shaderID) {

    // read in precompiled byte code... so it can then be compiled 8)
    const auto bytecodeSpan = shaders.at(t_shaderID);

    const uint8_t *bytecode = bytecodeSpan.data();
    size_t bytecodeSize = bytecodeSpan.size();
    // create gpu shader
    SDL_GPUShaderCreateInfo createInfo = {.code_size = bytecodeSize,
                                          .code = bytecode,
                                          .entrypoint = "main",
                                          .format = t_format,
                                          .stage = t_stage,
                                          .num_samplers = 0,
                                          .num_storage_textures = 0,
                                          .num_storage_buffers = 0,
                                          .num_uniform_buffers = 0,
                                          .props = 0};
    SDL_CreateGPUShader(t_device, &createInfo);
  }
  ~GPUShader() {
    if (m_shader != nullptr) {
      SDL_ReleaseGPUShader(m_device, m_shader);
    }
  }
};
