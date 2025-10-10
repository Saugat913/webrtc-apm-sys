use std::{
    env,
    fmt::format,
    fs,
    path::{Path, PathBuf},
    process::Command,
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

    println!("cargo:warning=Target: {} ({})", target_os, target_arch);

    build_webrtc_src(&out_dir, &webrtc_src, &target_os, &target_arch);
}

fn build_webrtc_src(out_dir: &Path, webrtc_src_dir: &Path, target_os: &str, target_arch: &str) {
    let build_dir = out_dir.join("build");

    // Check for previous build and clean it
    if build_dir.exists() {
        fs::remove_dir_all(&build_dir).ok();
    }

    // Create the required build folder
    fs::create_dir_all(&build_dir).expect("Failed to create build directory");

    println!(
        "cargo:warning=Buiding the source at {}",
        build_dir.display()
    );

    let build_type = if env::var("PROFILE").unwrap_or_default() == "release" {
        "release"
    } else {
        "debug"
    };

    // From readme of src: meson ../ command
    let mut meson_args = vec![format!("{}", webrtc_src_dir.display())];

    // Where to install library : OUT dir
    meson_args.push(format!("--prefix={}", out_dir.display()));

    meson_args.push(format!("--buildtype={}", build_type));

    println!("cargo:warning=Running: meson {}", meson_args.join(" "));

    let meson_status = Command::new("meson")
        .args(&meson_args)
        .current_dir(&build_dir)
        .status()
        .expect("Failed to run meson. Make sure meson is installed and in PATH");

    if !meson_status.success() {
        panic!("Meson configuration failed. Please check the error message above");
    }

    println!("cargo:warning=Meson configuration completed");

    println!("cargo:warning=Running ninja");

    let ninja_status = Command::new("ninja")
        .current_dir(&build_dir)
        .status()
        .expect("Failed to run ninja. Make sure ninja is installed and in PATH.");

    if !ninja_status.success() {
        panic!("Ninja build failed. Please check the error message above.");
    }

    println!("cargo:warning=Build completed");

    println!("cargo:warning=Installing the library");
    let ninja_install = Command::new("ninja")
        .arg("install")
        .current_dir(&build_dir)
        .status()
        .expect("Failed to installed the build library");

    if !ninja_install.success() {
        panic!("Ninja install failed.Please check the error message above.");
    }
    println!(
        "cargo:warning=Installed the library at {}",
        out_dir.display()
    );

    //Now lets link the library
    setup_library_linking(out_dir, target_os);
}
fn setup_library_linking(out_dir: &Path, target_os: &str) {
    let possible_path_for_library = vec![out_dir.join("lib"), out_dir.join("lib64")];

    let lib_extension = match target_os {
        "windows" => vec!["lib", "a"],
        _ => vec!["a", "so", "dylib"],
    };

    let lib_names = vec!["webrtc_audio_processing"];

    let mut found_lib = false;

    'outer: for path in possible_path_for_library {
        //if path doesnot exist no need to check
        if !path.exists() {
            continue;
        }

        //Ask cargo to register path for library search
        println!("cargo:rustc-link-search=native={}", path.display());
        for libname in &lib_names {
            for ext in &lib_extension {
                let combination_lib_name = path.join(format!("lib{}.{}", libname, ext));
                if combination_lib_name.exists() {
                    println!(
                        "cargo:warning=Found the library:{}",
                        combination_lib_name.display()
                    );
                    println!(
                        "cargo:rustc-link-lib=static={}",
                        combination_lib_name.display()
                    );
                    found_lib = true;
                    break 'outer;
                }
            }
        }
    }

    if !found_lib {
        println!("cargo:warning=No library found, attempting default linking");
        println!("cargo:rustc-link-lib=static=webrtc_audio_processing");
    }
}
