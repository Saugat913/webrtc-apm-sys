#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct webrtc_apm webrtc_apm_t;

webrtc_apm_t* webrtc_apm_create();
void webrtc_apm_free(webrtc_apm_t* w);

#ifdef __cplusplus
}
#endif
