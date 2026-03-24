#include "Audio.h"
#include "common.h"
#include <algorithm>
#include <string_view>
#include <utility>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

using enum EAudioError;
using Error = AudioError;

Error Audio::onInit() {
  m_context = ma_context{};
  if (ma_context_init(nullptr, 0, nullptr, &*m_context) != MA_SUCCESS) {
    return {INIT, "could not initialize audio context"};
  }

  m_playbackDeviceDescriptors.reserve(m_context->playbackDeviceInfoCount);
  if (ma_context_enumerate_devices(
          &*m_context,
          [](ma_context *, ma_device_type t_deviceType, const ma_device_info *t_info,
             void *t_userData) static -> uint32_t {
            if (t_deviceType == ma_device_type_playback) {
              auto *self = static_cast<Audio *>(t_userData);
              auto item = self->m_playbackDeviceDescriptors.insert(
                  self->m_playbackDeviceDescriptors.end(), *t_info);
              if (t_info->isDefault) {
                // this lets us assume default has an index of 0 so we don't have to store an
                // iterator
                self->m_playbackDeviceDescriptors.insert(self->m_playbackDeviceDescriptors.begin(),
                                                         *t_info);
              }
            }
            return MA_TRUE;
          },
          this) != MA_SUCCESS) {
    return {INIT, "could not enumerate audio devices"};
  }
  return {};
}

Error Audio::getShallowDevicesList(std::vector<std::string_view> &t_outputList) {
  PASS_ERROR(ensureInitialized<AudioError>(*this, "audio device list queried"))
  std::ranges::transform(m_playbackDeviceDescriptors.begin(), m_playbackDeviceDescriptors.end(),
                         t_outputList.begin(), [](auto &t_device) -> std::string_view {
                           return std::string_view(static_cast<const char *>(t_device.name));
                         });
  return {};
}

Error Audio::openDefaultDevice(){PASS_ERROR(openDevice(m_playbackDeviceDescriptors.begin()))}

Error Audio::openDevice(const AudioDevice &t_inputDevice) {
  PASS_ERROR(ensureInitialized<AudioError>(*this, "audio device open attempted"))

  auto deviceConfig = ma_device_config{
      // This block is equivalent to ma_device_config_init
      .deviceType = ma_device_type_playback,
      .sampleRate = {},
      .periods = {},
      .noClip = {},
      .dataCallback = {},
      .notificationCallback =
          [](auto *t_notification) {
            auto *audioContext = static_cast<Audio *>(t_notification->pDevice->pUserData);
            if (audioContext == nullptr) {
              logPassiveError(AudioError{
                  DEVICE_NOTIFY,
                  "ma_device_config's user data is no longer a pointer to its parent Audio *"});
              return;
            }
            switch (t_notification->type) {
#ifdef AUDIO_DEBUGGING
#define LOG_AUDIO_EVENT(EVENT)                                                                     \
  std::println("Playback " EVENT " on '{}'", t_notification->pDevice->playback.name)
            case ma_device_notification_type_started:
              LOG_AUDIO_EVENT("started");
              break;
            case ma_device_notification_type_stopped:
              LOG_AUDIO_EVENT("stopped");
              break;
            case ma_device_notification_type_interruption_began:
              LOG_AUDIO_EVENT("interrupted");
              break;
            case ma_device_notification_type_interruption_ended:
              LOG_AUDIO_EVENT("resumed from interruption");
              break;
            case ma_device_notification_type_unlocked:
              LOG_AUDIO_EVENT("unlocked");
              break;
            case ma_device_notification_type_rerouted:
              std::println("Playback rerouted from '{}' to '{}'",
                           audioContext->getCurrentDeviceName(),
                           t_notification->pDevice->playback.name);
#else
            case ma_device_notification_type_rerouted:
#endif
              audioContext->queueRestart();
              break;
            default:
              std::unreachable();
              break;
            }
          },
      .pUserData = this,
      .resampling = ma_resampler_config_init(
          ma_format_unknown, 0, 0, 0,
          ma_resample_algorithm_linear), // Format/channels/rate don't matter here. Lanczos?
      .playback =
          {
              .pDeviceID = &t_inputDevice->id,
          },

  };

  if (ma_device_init(&*m_context, &deviceConfig, m_device) != MA_SUCCESS) {
    return {DEVICE_INIT, "could not initialize audio device"};
  }
}

Error Audio::getCurrentDeviceName(std::string_view &t_output) {
  t_output = std::string_view(static_cast<const char *>(m_device->playback.name));
}

void Audio::queueRestart() { this->m_restartPending = true; }

Error Audio::closeDevice(const AudioDevice &t_inputDevice) { // TODO
}

Error Audio::onDestroy() {
  PASS_ERROR(closeDeviceInternal(m_device))
  ma_context_uninit(&*m_context);
  return {};
}

Error Audio::onUpdate() {
  if (m_restartPending) {
    PASS_ERROR(closeDeviceInternal(m_device))
    std::optional<AudioDevice> devicePreference{};
    PASS_ERROR(getDevicePreference(devicePreference))
    if (auto device = *devicePreference; devicePreference) {
      PASS_ERROR(openDevice(device))
    } else {
      PASS_ERROR(openDefaultDevice())
    }
  }
  return {};
}

Error Audio::closeDeviceInternal(ma_device *t_device) {
  if (t_device == nullptr) {
    return {DEVICE_UNINIT, "nullptr passed to closeDeviceInternal"};
  }
  ma_device_uninit(t_device);
}

// ----------
