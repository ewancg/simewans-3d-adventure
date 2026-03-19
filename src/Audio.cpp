#include "Audio.h"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"
using enum EAudioError;
using Error = AudioError;

Error Audio::onInit() {
  if (ma_context_init(nullptr, 0, nullptr, &m_context) != MA_SUCCESS) {
    return {INIT, "could not initialize audio context"};
  }

  m_playbackDeviceDescriptors.reserve(m_context.playbackDeviceInfoCount);
  if (ma_context_enumerate_devices(
          &m_context,
          [](ma_context *, ma_device_type t_deviceType, const ma_device_info *t_info,
             void *t_userData) -> uint32_t {
            if (t_deviceType == ma_device_type_playback) {
              auto *self = static_cast<Audio *>(t_userData);
              self->m_playbackDeviceDescriptors.push_back(*t_info);
            }
            return MA_TRUE;
          },
          nullptr) != MA_SUCCESS) {
    return {INIT, "could not enumerate audio devices"};
  }

  return {};
}
Error Audio::onDestroy() {
  ma_context_uninit(&m_context);
  return {};
}
Error Audio::onUpdate() { return {}; }

// ----------
