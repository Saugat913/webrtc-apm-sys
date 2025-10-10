# webrtc-apm-sys

Low-level Rust FFI bindings to the WebRTC Audio Processing module.
This crate provides raw, unsafe C bindings to the WebRTC Audio Processing (APM) library via a small C wrapper.

⚠️ This is a -sys crate: it only exposes unsafe bindings. For a safe, ergonomic API, you should build a higher-level crate on top of this.

## Features
- Minimal C wrapper around WebRTC’s AudioProcessing API

## Building

This crate relies on building the WebRTC APM module from source.
The build.rs script:
- Locates/compiles WebRTC

- Links against the produced static library (libwebrtc_audio_processing.a)

- Exposes the  wrapper (wrapper.cpp)

## Requirements:

- C++ compiler (Clang/GCC/MSVC)

- Meson + Ninja (for building WebRTC)

## Status

✅ Basic bindings (create, free)

🚧 Missing advanced controls (process,AGC, NS, Echo Cancellation, etc.)

Contributions welcome!


## Special Thanks

This crate uses [webrtc-audio-processing](https://github.com/cross-platform/webrtc-audio-processing.git)
 as a submodule for building the native WebRTC Audio Processing library.
Huge thanks to the original author(s) for making their work available. 🙏


If you would like to contribute (e.g., add advanced controls or safe Rust wrappers), please feel free to open a pull request or issue.