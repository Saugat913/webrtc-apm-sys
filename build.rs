use std::{
    env, fs,
    path::{Path, PathBuf},
    process::Command,
};

const BUILD_LIBRARY_NAME: &str = "webrtc-audio-processing-2";
const SOURCE_GIT_URL: &str = "https://gitlab.freedesktop.org/pulseaudio/webrtc-audio-processing.git";

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
        git clone {SOURCE_GIT_URL} webrtc-src",
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

    build_webrtc_src(&out_dir, &webrtc_src, &target_os);
    build_wrapper(&out_dir, &manifest_dir, &target_os);
    generate_bindings(&out_dir, &manifest_dir);
}

fn build_webrtc_src(out_dir: &Path, webrtc_src_dir: &Path, target_os: &str) {
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
    let mut meson_args = vec![
        "setup".to_string(),
        format!("{}", build_dir.display()),
        "-Dcpp_args=-DNOMINMAX".to_string(),
        "-Dcpp_std=c++20".to_string(),
    ];

    // Where to install library : OUT dir
    meson_args.push(format!("--prefix={}", out_dir.display()));

    meson_args.push(format!("--buildtype={}", build_type));

    println!(
        "cargo:warning=Running: meson {} at {}",
        meson_args.join(" "),
        &out_dir.display()
    );

    let meson_status = Command::new("meson")
        .args(&meson_args)
        .current_dir(&webrtc_src_dir)
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

    let lib_names = vec![BUILD_LIBRARY_NAME];

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
                let combination_lib_name = format!("{}.{}", libname, ext);
                let combination_lib_name_path = path.join(&combination_lib_name);
                if combination_lib_name_path.exists() {
                    println!(
                        "cargo:warning=Found the library:{}",
                        combination_lib_name_path.display()
                    );
                    println!("cargo:rustc-link-lib=static={}", libname);
                    found_lib = true;
                    break 'outer;
                }
            }
        }
    }

    if !found_lib {
        println!("cargo:warning=No library found, attempting default linking");
        println!("cargo:rustc-link-lib=static={}", BUILD_LIBRARY_NAME);
    }
}
fn build_wrapper(out_dir: &Path, manifest_dir: &Path, target_os: &str) {
    println!("cargo:warning=Building C++ wrapper");

    let wrapper_cpp = manifest_dir.join("wrapper.cpp");
    if !wrapper_cpp.exists() {
        println!("cargo:warning=Wrapper source not found, skipping wrapper build");
        return;
    }

    let mut build = cc::Build::new();
    build
        .cpp(true)
        .file(&wrapper_cpp)
        .include(out_dir.join("include").join("webrtc-audio-processing-2"))
        .include(out_dir.join("include"))
        .flag("-DWEBRTC_AUDIO_PROCESSING_ONLY_BUILD")
        .flag_if_supported("/std:c++20");

    build.compile("wrapper");

    println!("cargo:warning=C++ wrapper compiled successfully");
}

fn generate_bindings(out_dir: &Path, manifest_dir: &Path) {
    println!("cargo:warning=Generating Rust bindings for C wrapper");

    let wrapper_header = manifest_dir.join("wrapper.h");

    if !wrapper_header.exists() {
        println!("cargo:warning=Wrapper header not found, skipping bindgen");
        return;
    }

    let bindings = bindgen::Builder::default()
        .header(wrapper_header.to_string_lossy().to_string())
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .generate_comments(true)
        .generate()
        .expect("Unable to generate bindings");

    let out_path = out_dir.join("bindings.rs");
    bindings
        .write_to_file(&out_path)
        .expect("Couldn't write bindings!");

    println!("cargo:warning=Bindings written to: {}", out_path.display());
}
