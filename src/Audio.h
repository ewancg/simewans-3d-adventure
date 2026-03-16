#pragma once
#include "Subsystem.h"

#ifndef AUDIO_DEBUGGING
#define AUDIO_DEBUGGING false
#endif

#define ERROR_ENTRIES(E)                                                                           \
  E(INIT, "initializing the audio subsystem")                                                      \
  E(DEVICE_INIT, "initializing the audio device")

DEFINE_DERIVED_ERROR_TYPES(Audio, ESubsystemError::END, SubsystemError, ESubsystemError,
                           ERROR_ENTRIES);
#undef ERROR_ENTRIES

class Audio : public Subsystem<AudioError> {
public:
  using Error = AudioError;
  Error onInit();
  Error onDestroy();
  Error onUpdate();

private:
  DEFINE_PROPERTY(bool, m_isMuted, getMutedState, setMutedState, 0);
  DEFINE_PROPERTY(float, m_masterVolume, getVolume, setVolume, 0.75F);
};
