#include "Audio.h"
#include "common.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>
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
              auto  item = self->m_playbackDeviceDescriptors.insert(
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

Error Audio::openDefaultDevice() { PASS_ERROR(openDevice(m_playbackDeviceDescriptors.begin())) }

template <uint8_t N, bool DiscardLSB = false> // false = keep lowest N bytes (for s24)
std::array<uint8_t, N> to_bytes(int32_t value) {
  static_assert(N >= 1 && N <= 4);

  uint32_t u = static_cast<uint32_t>(value); // preserves two's-complement bits

  if constexpr (!DiscardLSB) {
    u &= (0xFFFFFFFFu >> (32 - N * 8)); // keep lowest N bytes
  } else {
    u >>= (32 - N * 8); // keep highest N bytes
  }

  std::array<uint8_t, N> bytes{};
  for (uint8_t i = 0; i < N; ++i) { // little-endian (standard for audio)
    bytes[i] = static_cast<uint8_t>((u >> (i * 8)) & 0xFF);
  }
  return bytes;
}

template <typename T, uint8_t P>
void fillWaves(void *t_out, size_t t_size, std::optional<T> t_max) {
  T basis{};
  if constexpr (std::is_signed_v<T>) {
    basis = 0;
  } else {
    basis = (t_max ? *t_max : std::numeric_limits<T>::max()) / 2;
  }

  if constexpr (P == 0) {
    // native full-size types (u8/s16/s32/f32) — unchanged
    auto *data = static_cast<T *>(t_out);
    for (size_t i = 0; i < t_size / sizeof(T); ++i) {
      data[i] = basis + static_cast<T>(std::sin(static_cast<double>(i)));
    }
  } else {
    // packed formats (s24 = P=3, or any future N-byte format)
    auto *data = static_cast<uint8_t *>(t_out);
    for (size_t written = 0; written < t_size; written += P) {
      T sample = basis + static_cast<T>(std::sin(static_cast<double>(written / P)));

      auto bytes = to_bytes<P, false>(static_cast<int32_t>(sample)); // false = discard MSB

      for (uint8_t b = 0; b < P; ++b) {
        data[written + b] = bytes[b];
      }
    }
  }
}

Error Audio::openDevice(const AudioDevice &t_inputDevice) {
  PASS_ERROR(ensureInitialized<AudioError>(*this, "audio device open attempted"))
  auto deviceConfig = ma_device_config{
      // This block is equivalent to ma_device_config_init
      .deviceType = ma_device_type_playback,
      .sampleRate = {},
      .periods    = {},
      .noClip     = {},
      .dataCallback =
          [](ma_device *t_device, void *t_output, const void *, ma_uint32 t_frameCount) {
            auto             *audioContext = static_cast<Audio *>(t_device->pUserData);
            const static auto bail = [&]() { audioContext->m_dataCallbackState.error = true; };
            auto              deviceInfo = ma_device_info{};
            if (ma_device_get_info(t_device, ma_device_type_playback, &deviceInfo) != MA_SUCCESS) {
              bail();
              return;
            }

            switch (t_device->playback.format) {
#define FORMAT_CTYPE_CASE(FORMAT, MIX_TYPE, MAX_VAL, MIX_BPS)                                      \
  case ma_format_##FORMAT:                                                                         \
    fillWaves<MIX_TYPE, MIX_BPS>(t_output, t_frameCount, MAX_VAL);                                 \
    break;
              FORMAT_CTYPE_CASE(u8, uint8_t, std::nullopt, 0)
              FORMAT_CTYPE_CASE(s16, int16_t, std::nullopt, 0)
              FORMAT_CTYPE_CASE(s24, int32_t, 0x00FFFFFF, 3)
              FORMAT_CTYPE_CASE(s32, int32_t, std::nullopt, 0)
              FORMAT_CTYPE_CASE(f32, double, std::nullopt, 0)
            case ma_format_count:
              [[fallthrough]];
            case ma_format_unknown:
              std::unreachable();
            }

            // Sine wave
          },
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
      .pUserData  = this,
      .resampling = ma_resampler_config_init(
          ma_format_unknown, 0, 0, 0,
          ma_resample_algorithm_linear), // Format/channels/rate don't matter here. Lanczos?
      .playback =
          {
              .pDeviceID = &t_inputDevice->id,
              .format    = ma_format_unknown,
          },

  };

  if (auto err = ma_device_init(&*m_context, &deviceConfig, m_device); err != MA_SUCCESS) {
    return {DEVICE_INIT, ma_result_description(err)};
  }
  m_restartPending = false;
  AUDIO_DEBUG_LOG("audio device opened");
}

Error Audio::getCurrentDeviceName(std::string_view &t_output) {
  t_output = std::string_view(static_cast<const char *>(m_device->playback.name));
  AUDIO_DEBUG_LOG("audio device opened {}", t_output);
}

void Audio::queueRestart() { this->m_restartPending = true; }

Error Audio::closeDevice(const AudioDevice &t_inputDevice) { // TODO
}

Error Audio::onDestroy() {
  // PASS_ERROR(closeDeviceInternal(m_device))
  return {};
}

Error Audio::onUpdate() {
  return {}; // remove later
  if (m_restartPending) {
    if (m_device != nullptr) {
      PASS_ERROR(closeDeviceInternal(m_device))
    }
    std::optional<AudioDevice> devicePreference{};
    PASS_ERROR(getDevicePreference(devicePreference))
    if (auto device = *devicePreference; devicePreference) {
      PASS_ERROR(openDevice(device))
    } else {
      PASS_ERROR(openDefaultDevice())
    }
    m_restartPending = false;
  }
  return {};
}

Error Audio::closeDeviceInternal(ma_device *t_device) {
  if (t_device == nullptr) {
    return {DEVICE_UNINIT, "nullptr passed to closeDeviceInternal"};
  }
  t_device = nullptr;
  ma_device_uninit(t_device);
}

// ----------
