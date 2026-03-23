#pragma once
#include "Subsystem.h"
#include "miniaudio/miniaudio.h"

#if defined(AUDIO_DEBUGGING) && AUDIO_DEBUGGING != false
#define AUDIO_DEBUGGING
#endif

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing the audio subsystem")                                                      \
  E(DEVICE_INIT, "initializing the audio device")                                                  \
  E(DEVICE_UNINIT, "uninitializing the audio device")                                              \
  E(DEVICE_NOTIFY, "issuing an audio device notification")
DEFINE_DERIVED_ERROR_TYPES(Audio, Subsystem, ERROR_ENTRIES);
#undef ERROR_ENTRIES

/// This class handles playback only as that's all the game cares about for now
class Audio : public Subsystem<AudioError> {
  SUBSYSTEM(Audio)
public:
  using AudioDevice = std::vector<ma_device_info>::iterator;

  Error getShallowDevicesList(
      std::vector<std::string_view>
          &t_outputList); // index & string is usually all that's required externally, default is 0
  Error openDefaultDevice();
  Error openDevice(const AudioDevice &t_inputDevice);
  Error closeDevice(const AudioDevice &t_inputDevice);

  Error getCurrentDeviceName(std::string_view &t_output);
  void queueRestart();

private:
  ma_context m_context;
  ma_device *m_device{};
  std::vector<ma_device_info> m_playbackDeviceDescriptors;

  bool m_restartPending{};

  static Error closeDeviceInternal(ma_device *t_device);

  DEFINE_REF_PROPERTY(std::optional<AudioDevice>, m_devicePreference, getDevicePreference,
                      setDevicePreference, std::nullopt);
  DEFINE_PROPERTY(bool, m_isMuted, getMutedState, setMutedState, 0);
  DEFINE_PROPERTY(float, m_masterVolume, getVolume, setVolume, 0.75F);
};
