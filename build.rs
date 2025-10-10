use std::{
    env,
    path::{Path, PathBuf},
};

fn main() {
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    // Absolute path of your crate root directory where Cargo.toml is present
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    // Webrtc source code dir
    let webrtc_src = manifest_dir.join("webrtc-src");

    // Why printing ? This is not printing it is special command for cargo build to rebuild if something changed
    println!("cargo:rerun-if-changed=build.rs");
    println!("cargo:rerun-if-changed={}", webrtc_src.display());

    // check source code existence
    if !webrtc_src.exists() {
        panic!(
            "Webrtc source not found at:{}. Please clone:\
        git clone https://github.com/cross-platform/webrtc-audio-processing.git webrtc-src",
            webrtc_src.display()
        );
    }
    println!(
        "cargo:warning=Building WebRTC from: {}",
        webrtc_src.display()
    );

    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();

    let target_arch = env::var("CARGO_CFG_TARGET_ARCH").unwrap();

    print!("cargo:warning=Target: {} ({})", target_os, target_arch);

    build_webrtc_src(&out_dir, &webrtc_src, &target_os, &target_arch);
}

fn build_webrtc_src(out_dir: &Path, webrtc_src_dir: &Path, target_os: &str, target_arch: &str) {
    
}
