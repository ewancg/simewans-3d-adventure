#pragma once
#include "Subsystem.h"

#ifndef AUDIO_DEBUGGING
#define AUDIO_DEBUGGING false
#endif

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing the audio subsystem")                                                      \
  E(DEVICE_INIT, "initializing the audio device")
DEFINE_DERIVED_ERROR_TYPES(Audio, Subsystem, ERROR_ENTRIES);
#undef ERROR_ENTRIES

class Audio : public Subsystem<AudioError> {
  SUBSYSTEM(Audio)
private:
  static uint32_t enumerateDevicesInternal(ma_context *t_context, ma_device_type t_deviceType,
                                           const ma_device_info *t_info, void *);
  ma_context m_context;
  std::vector<ma_device_info> m_playbackDeviceDescriptors;

  DEFINE_PROPERTY(bool, m_isMuted, getMutedState, setMutedState, 0);
  DEFINE_PROPERTY(float, m_masterVolume, getVolume, setVolume, 0.75F);
};
