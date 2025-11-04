//Made by Han_feng

#![allow(non_camel_case_types)]

mod libs;

use std::fs::File;
use std::sync::atomic::Ordering;
use crate::libs::utils::LOG_LEVEL;

pub use libs::utils::Status_code;
#[cfg(debug_assertions)]
pub use libs::utils::Log_level;

#[cfg(debug_assertions)]
#[unsafe(no_mangle)]
pub extern "C" fn log_init(log_level: Log_level){
    LOG_LEVEL.store(log_level as u32, Ordering::Relaxed);
    
    let log_file = File::create("log_file.log").unwrap();
    env_logger::Builder::from_default_env()
        .target(env_logger::Target::Pipe(Box::new(log_file)))
        .init();
}