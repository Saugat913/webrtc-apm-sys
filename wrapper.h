#pragma once
#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C"
{
#endif
    //========================== Handles ===============================
    typedef struct WebRtcAudioProcessing *WebRtcAudioProcessingHandle;
    typedef struct WebRtcStreamConfig *WebRtcStreamConfigHandle;

    //======================== Error Types ===============================
    typedef enum
    {
        WEBRTC_APM_NO_ERROR = 0,
        WEBRTC_APM_UNSPECIFIED_ERROR = -1,
        WEBRTC_APM_BAD_PARAMETER_ERROR = -6,
        WEBRTC_APM_BAD_SAMPLE_RATE_ERROR = -7,
        WEBRTC_APM_NULL_POINTER_ERROR
    } WebRtcApmError;

    //======================== Module Configuration ===========================

    // Noise Suppression
    typedef enum
    {
        // LOW: Mild reduction (less speech distortion).
        WEBRTC_APM_NS_LOW,
        // MODERATE: Balanced setting (default).
        WEBRTC_APM_NS_MODERATE,
        // HIGH: Stronger suppression.
        WEBRTC_APM_NS_HIGH,
        // VERY_HIGH: Maximum suppression (may distort voice).
        WEBRTC_APM_NS_VERY_HIGH
    } WebRtcApmNoiseSuppressionLevel;

    typedef struct
    {
        /* Noise Suppression */
        bool enable_noise_suppression;
        WebRtcApmNoiseSuppressionLevel ns_level;

        /* Echo Cancellation */
        bool enable_echo_cancellation;
        bool echo_mobile_mode; /* Lightweight mode for mobile */

        /* High-Pass Filter (removes low-frequency rumble) */
        bool enable_high_pass_filter;

        /* Automatic Gain Control */
        bool enable_analog_gain_control;
        int agc_target_level_dbfs;   /* Target level in dB (-31 to 0, default -3) */
        int agc_compression_gain_db; /* Compression gain (0-90, default 9) */
        bool agc_enable_limiter;     /* Hard limit at target level */
    } WebRtcAudioProcessingConfig;

    // Create a new StreamConfig with specified sample rate and number of channels.
    WebRtcStreamConfigHandle webrtc_apm_stream_config_create(int sample_rate_hz, size_t num_channels);
    // As name suggest just destroy and clean up the config handle
    void webrtc_apm_stream_config_destroy(WebRtcStreamConfigHandle config);

    // Create a new AudioProcessing instance with default configuration.
    WebRtcAudioProcessingHandle webrtc_apm_create(void);

    // Destroy an AudioProcessing instance.
    void webrtc_apm_destroy(WebRtcAudioProcessingHandle apm);

    // Initialize the AudioProcessing instance with default parameters.
    WebRtcApmError webrtc_apm_initialize(WebRtcAudioProcessingHandle apm);

    WebRtcApmError webrtc_apm_apply_config(WebRtcAudioProcessingHandle processor,
                                           const WebRtcAudioProcessingConfig *config);

    // Process a frame of capture audio (int16).
    WebRtcApmError webrtc_apm_process_stream_int16(
        WebRtcAudioProcessingHandle apm,
        WebRtcStreamConfigHandle input_config,
        WebRtcStreamConfigHandle output_config,
        const int16_t *src,
        int16_t *dest);

    // Process a reverse stream frame (int16) for echo cancellation reference.
    // Use this function when you want to use the echo cancellation and feed
    // the data of the speaker to it.
    WebRtcApmError webrtc_apm_process_reverse_stream_int16(
        WebRtcAudioProcessingHandle apm,
        WebRtcStreamConfigHandle input_config,
        WebRtcStreamConfigHandle output_config,
        const int16_t *src,
        int16_t *dest);

    // NOTE: Audio data of float type for processing should be array of pointers to deinterleaved float audio channels. What does it mean ? I also dont know about it is like system of arranging the audio data:
    // Interleaved: L R L R L R
    // Deinterleaved :if 2 channel then data is splitted as array of array.
    //  for example:
    //          src[2]
    //          src[0]-> Left channel [ frame0,frame1,...]
    //          src[1]-> Right channel [ frame0,frame1,...]

    // Most of the audio backend provide the interleaved data so just convert the data provided type into int16 type and process. If backend provide deinterleaved data use float version.

    // Process a frame of capture audio (float).
    WebRtcApmError webrtc_apm_process_stream_float32(
        WebRtcAudioProcessingHandle apm,
        WebRtcStreamConfigHandle input_config,
        WebRtcStreamConfigHandle output_config,
        const float *const *src,
        float *const *dest);

    // Process a reverse stream frame (float) for echo cancellation reference.
    // Use this function when you want to use the echo cancellation and feed
    // the data of the speaker to it.
    WebRtcApmError webrtc_apm_process_reverse_stream_float32(
        WebRtcAudioProcessingHandle apm,
        WebRtcStreamConfigHandle input_config,
        WebRtcStreamConfigHandle output_config,
        const float *const *src,
        float *const *dest);

    // Set the stream delay in milliseconds (for echo cancellation).
    // Why it matters ?
    // - Echo cancellation needs to know how long it takes for the sound you play (speaker output) to come back into the mic input.
    // - Without this info, AEC can't properly align the reference and mic signal → echo removal becomes inaccurate or fails.

    // Some of the estimated delays according to os :
    // | Platform          | Typical Delay (ms) |
    // | ----------------- | ------------------ |
    // | Windows (WASAPI)  | 80–150             |
    // | macOS / CoreAudio | 30–60              |
    // | Android           | 100–250            |
    // | iOS               | 40–80              |

    WebRtcApmError webrtc_apm_set_streamdelay_ms(WebRtcAudioProcessingHandle apm,
                                                 int delay_ms);

#ifdef __cplusplus
}
#endif
