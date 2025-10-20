#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

#[link(name = "webrtc_audio_processing", kind = "static")]
unsafe extern "C" {}

// Include generated bindings
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
