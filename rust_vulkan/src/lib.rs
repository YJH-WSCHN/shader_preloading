//Made by Han_feng

#![allow(non_camel_case_types)]
#![allow(unused_unsafe)]

mod libs;

use std::ffi::{c_char, c_uint, c_void};
use libs::utils::C_char_extension;

cfg_if::cfg_if!{
    if #[cfg(debug_assertions)] {
        use libs::utils::{Log_level, LOG_LEVEL};
        use std::path::Path;
        use std::fs::{File, create_dir_all};
        use std::io::Write;
        use log::{Level, LevelFilter};
    }
}

pub use libs::utils::Status_code;
pub use libs::vulkan_application::{Vulkan_application, Window};

#[cfg(debug_assertions)]
#[unsafe(no_mangle)]
pub extern "C" fn log_init(log_file: *const c_char, log_level: c_uint) -> Status_code{
    unsafe {
        LOG_LEVEL = Log_level(log_level as u32);

        if !LOG_LEVEL.is_empty(){
            env_logger::Builder::from_default_env()
                .target(if log_file.is_null(){
                    env_logger::Target::Stdout
                }
                else {
                    let log_path = Path::new(log_file.to_str());
                    if !log_path.exists() && let Some(log_dir) = log_path.parent() && create_dir_all(log_dir).is_err(){
                        return Status_code::Failure;
                    }
                    env_logger::Target::Pipe(Box::new(File::create(log_path).unwrap()))
                })
                .format(|buf, record| {
                    writeln!(buf, "[{}]{}", record.level(), record.args())
                })
                .filter_level(LevelFilter::Trace)
                .init();

            save_log!(Log_level::GENERAL, Level::Info, "Successfully initialized the logger");
        }

        Status_code::Success
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn get_vulkan_application(window_handle: *mut c_void, width: c_uint, height: c_uint, vulkan_path: *const c_char, vulkan_application: *mut *mut Vulkan_application) -> Status_code{
    let window = match Window::new(window_handle, width as u32, height as u32){
        Ok(window) => window,
        Err(code) => return code,
    };

    let application = Box::new(match Vulkan_application::new(window, if vulkan_path.is_null() {None} else {Some(vulkan_path.to_str())}) {
        Ok(application) => application,
        Err(code) => return code,
    });

    unsafe { *vulkan_application = Box::into_raw(application) }

    Status_code::Success
}

#[unsafe(no_mangle)]
pub extern "C" fn draw_frame(vulkan_application: *mut Vulkan_application) -> Status_code {
    unsafe {
        match (*vulkan_application).draw_frame(){
            Ok(_) => Status_code::Success,
            Err(code) => code,
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn destroy_vulkan_application(vulkan_application: *mut Vulkan_application){
    unsafe {
        let _ = Box::from_raw(vulkan_application);
    }
}