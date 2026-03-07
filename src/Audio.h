#pragma once

#define ERRORS(E)                                                                                  \
  E(INIT, "initializing the audio subsystem")                                                      \
  E(DEVICE_INIT, "initializing the audio device")
DEFINE_ERROR_TYPES(Audio, ERRORS);
#undef ERRORS

class Audio : public Subsystem<AudioError> {
  using Error = AudioError;

public:
  Error onInit();
  Error onDestroy();
};
