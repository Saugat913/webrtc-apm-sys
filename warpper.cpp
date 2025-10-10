#include "modules/audio_processing/include/audio_processing.h"
#include <stdlib.h>

extern "C" {

typedef struct webrtc_apm {
    webrtc::AudioProcessing* apm;
}webrtc_apm_t;


// Create instance
webrtc_apm_t* webrtc_apm_create() {
    webrtc_apm_t* w = (webrtc_apm_t*)malloc(sizeof(webrtc_apm_t));
    if (!w) return NULL;
    w->apm = webrtc::AudioProcessingBuilder().Create();
    return w;
}

// Free instance
void webrtc_apm_free(webrtc_apm_t* w) {
    if (w) {
        delete w->apm;
        free(w);
    }
}

}