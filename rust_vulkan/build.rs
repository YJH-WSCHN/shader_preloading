//Made by Han_feng

use std::error::Error;
use std::{env, fs};
use std::io::Write;
use std::path::{Path, PathBuf};
use shaderc::{CompileOptions, Compiler, OptimizationLevel, ShaderKind};

fn main() -> Result<(), Box<dyn Error>> {
    //Shader pre-compile
    let shader_compiler = Compiler::new()?;
    let mut compile_options = CompileOptions::new()?;
    compile_options.set_optimization_level(OptimizationLevel::Performance);

    let result_path = Path::new(&env::var("OUT_DIR")?).join("shaders");
    if !result_path.exists() {
        fs::create_dir_all(&result_path).unwrap();
    }
    for (i, path) in fs::read_dir(Path::new("shaders"))?.map(|entry| entry.unwrap().path()).enumerate() {
        let source = fs::read_to_string(&path)?;
        let compile_result = shader_compiler.compile_into_spirv(
            source.as_str(),
            match path.extension().and_then(|s| s.to_str()).unwrap_or("") {
                "vert" => ShaderKind::Vertex,
                "frag" => ShaderKind::Fragment,
                _ => continue,
            },
            path.to_str().unwrap(),
            "main",
            Some(&compile_options),
        )?;

        let mut shader_file = fs::File::create(result_path.join(&format!("{}.spv", path.file_stem().and_then(|s| s.to_str()).unwrap_or(&format!("Unknown {}", i)))))?;
        shader_file.write_all(compile_result.as_binary_u8())?;
    };

    println!("cargo:rerun-if-changed=shaders");

    //C Header generate
    let target_path = PathBuf::from(env::var("OUT_DIR")?)  //out
        .parent().unwrap() //hash
        .parent().unwrap() //build
        .parent().unwrap() //target dir
        .join("rust_vulkan_lib.h");

    let config = cbindgen::Config{
        language: cbindgen::Language::C,
        header: Some("//Made by Han_feng".to_string()),
        include_guard: Some("RUST_VULKAN_LIB_H".to_string()),
        ..Default::default()
    };

    cbindgen::Builder::new()
        .with_src(PathBuf::from(env::var("CARGO_MANIFEST_DIR")?).join("src/lib.rs"))
        .with_config(config)
        .generate()?
        .write_to_file(target_path);

    println!("cargo:rerun-if-changed=src/lib.rs");
    Ok(())
}