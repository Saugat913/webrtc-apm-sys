#include "modules/audio_processing/include/audio_processing.h"
#include "wrapper.h"
#include <stdlib.h>

extern "C"
{

    struct WebRtcAudioProcessing
    {
        rtc::scoped_refptr<webrtc::AudioProcessing> apm;
        webrtc::AudioProcessing::Config current_config;
    };

    struct WebRtcStreamConfig
    {
        webrtc::StreamConfig config;
    };

    WebRtcStreamConfigHandle webrtc_apm_stream_config_create(int sample_rate_hz, size_t num_channels)
    {
        try
        {
            auto *config = new WebRtcStreamConfig();
            config->config = webrtc::StreamConfig(sample_rate_hz, num_channels);
            return config;
        }
        catch (...)
        {
            return nullptr;
        }
    }

    void webrtc_apm_stream_config_destroy(WebRtcStreamConfigHandle config)
    {
        delete static_cast<WebRtcStreamConfig *>(config);
    }

    WebRtcAudioProcessingHandle webrtc_apm_create(void)
    {
        try
        {
            auto *apm_wrapper = new WebRtcAudioProcessing();
            apm_wrapper->apm = webrtc::AudioProcessingBuilder().Create();
            if (!apm_wrapper->apm)
            {
                delete apm_wrapper;
                return nullptr;
            }
            return apm_wrapper;
        }
        catch (...)
        {
            return nullptr;
        }
    }

    void webrtc_apm_destroy(WebRtcAudioProcessingHandle apm)
    {
        if (apm)
        {
            auto *a = static_cast<WebRtcAudioProcessing *>(apm);
            a->apm = nullptr;
            delete a;
        }
    }

    WebRtcApmError webrtc_apm_initialize(WebRtcAudioProcessingHandle apm)
    {
        if (!apm)
        {
            return WEBRTC_APM_NULL_POINTER_ERROR;
        }
        try
        {
            auto *ap = static_cast<WebRtcAudioProcessing *>(apm);
            int result = ap->apm->Initialize();
            if (result == 0)
                return WEBRTC_APM_NO_ERROR;
            else
                return WEBRTC_APM_UNSPECIFIED_ERROR;
        }
        catch (const std::exception &e)
        {
            return WEBRTC_APM_UNSPECIFIED_ERROR;
        }
    }

    WebRtcApmError webrtc_apm_apply_config(WebRtcAudioProcessingHandle processor,
                                           const WebRtcAudioProcessingConfig *config)
    {
        if (!processor || !config)
        {
            return WEBRTC_APM_NULL_POINTER_ERROR;
        }

        try
        {
            auto *ap = static_cast<WebRtcAudioProcessing *>(processor);

            /* Noise Suppression Configuration */
            ap->current_config.noise_suppression.enabled =
                config->enable_noise_suppression;
            ap->current_config.noise_suppression.level = static_cast<webrtc::AudioProcessing::Config::NoiseSuppression::Level>(config->ns_level);

            /* Echo Cancellation Configuration */
            ap->current_config.echo_canceller.enabled =
                config->enable_echo_cancellation;
            ap->current_config.echo_canceller.mobile_mode =
                config->echo_mobile_mode;

            /* High-Pass Filter Configuration */
            ap->current_config.high_pass_filter.enabled =
                config->enable_high_pass_filter;

            /* Gain Controller 1 Configuration */
            ap->current_config.gain_controller1.enabled =
                config->enable_analog_gain_control;
            ap->current_config.gain_controller1.target_level_dbfs =
                config->agc_target_level_dbfs;
            ap->current_config.gain_controller1.compression_gain_db =
                config->agc_compression_gain_db;
            ap->current_config.gain_controller1.enable_limiter =
                config->agc_enable_limiter;

            /* Apply the configuration */
            ap->apm->ApplyConfig(ap->current_config);

            return WEBRTC_APM_NO_ERROR;
        }
        catch (const std::exception &e)
        {
            return WEBRTC_APM_UNSPECIFIED_ERROR;
        }
    }

    WebRtcApmError webrtc_apm_process_stream_int16(
        WebRtcAudioProcessingHandle apm,
        WebRtcStreamConfigHandle input_config,
        WebRtcStreamConfigHandle output_config,
        const int16_t *src,
        int16_t *dest)
    {
        if (!apm || !input_config || !output_config || !src || !dest)
        {
            return WEBRTC_APM_NULL_POINTER_ERROR;
        }

        try
        {
            auto *ap = static_cast<WebRtcAudioProcessing *>(apm);
            auto *in_cfg = static_cast<WebRtcStreamConfig *>(input_config);
            auto *out_cfg = static_cast<WebRtcStreamConfig *>(output_config);

            int result = ap->apm->ProcessStream(src, in_cfg->config,
                                                out_cfg->config, dest);
            return static_cast<WebRtcApmError>(result);
        }
        catch (const std::exception &e)
        {
            return WEBRTC_APM_UNSPECIFIED_ERROR;
        }
    }

    WebRtcApmError webrtc_apm_process_reverse_stream_int16(
        WebRtcAudioProcessingHandle apm,
        WebRtcStreamConfigHandle input_config,
        WebRtcStreamConfigHandle output_config,
        const int16_t *src,
        int16_t *dest)
    {

        if (!apm || !input_config || !output_config || !src || !dest)
        {
            return WEBRTC_APM_NULL_POINTER_ERROR;
        }

        try
        {
            auto *ap = static_cast<WebRtcAudioProcessing *>(apm);
            auto *in_cfg = static_cast<WebRtcStreamConfig *>(input_config);
            auto *out_cfg = static_cast<WebRtcStreamConfig *>(output_config);

            int result = ap->apm->ProcessReverseStream(src, in_cfg->config,
                                                       out_cfg->config, dest);
            return static_cast<WebRtcApmError>(result);
        }
        catch (const std::exception &e)
        {
            return WEBRTC_APM_UNSPECIFIED_ERROR;
        }
    }

    WebRtcApmError webrtc_apm_process_stream_float32(
        WebRtcAudioProcessingHandle apm,
        WebRtcStreamConfigHandle input_config,
        WebRtcStreamConfigHandle output_config,
        const float *const *src,
        float *const *dest)
    {
        if (!apm || !input_config || !output_config || !src || !dest)
        {
            return WEBRTC_APM_NULL_POINTER_ERROR;
        }

        try
        {
            auto *ap = static_cast<WebRtcAudioProcessing *>(apm);
            auto *in_cfg = static_cast<WebRtcStreamConfig *>(input_config);
            auto *out_cfg = static_cast<WebRtcStreamConfig *>(output_config);

            int result = ap->apm->ProcessStream(src, in_cfg->config,
                                                out_cfg->config, dest);
            return static_cast<WebRtcApmError>(result);
        }
        catch (const std::exception &e)
        {
            return WEBRTC_APM_UNSPECIFIED_ERROR;
        }
    }

    WebRtcApmError webrtc_apm_process_reverse_stream_float32(
        WebRtcAudioProcessingHandle apm,
        WebRtcStreamConfigHandle input_config,
        WebRtcStreamConfigHandle output_config,
        const float *const *src,
        float *const *dest)
    {
        if (!apm || !input_config || !output_config || !src || !dest)
        {
            return WEBRTC_APM_NULL_POINTER_ERROR;
        }

        try
        {
            auto *ap = static_cast<WebRtcAudioProcessing *>(apm);
            auto *in_cfg = static_cast<WebRtcStreamConfig *>(input_config);
            auto *out_cfg = static_cast<WebRtcStreamConfig *>(output_config);

            int result = ap->apm->ProcessReverseStream(src, in_cfg->config,
                                                       out_cfg->config, dest);
            return static_cast<WebRtcApmError>(result);
        }
        catch (const std::exception &e)
        {
            return WEBRTC_APM_UNSPECIFIED_ERROR;
        }
    }

    WebRtcApmError webrtc_apm_set_streamdelay_ms(WebRtcAudioProcessingHandle apm,
                                                 int delay_ms)
    {
        if (!apm)
        {
            return WEBRTC_APM_NULL_POINTER_ERROR;
        }

        try
        {
            auto *ap = static_cast<WebRtcAudioProcessing *>(apm);
            int result = ap->apm->set_stream_delay_ms(delay_ms);
            return static_cast<WebRtcApmError>(result);
        }
        catch (const std::exception &e)
        {
            return WEBRTC_APM_UNSPECIFIED_ERROR;
        }
    }
}