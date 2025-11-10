//Made by Han_feng

use std::ffi::{c_char, CStr};

cfg_if::cfg_if! {
    if #[cfg(debug_assertions)] {
        use log::Level;
        use std::collections::VecDeque;
        use std::io::Write;
        use std::sync::{Arc, Mutex};
    }
}

//Statics
#[cfg(debug_assertions)]
pub static mut LOG_LEVEL: u64 = 0;

//Types
pub type Vulkan_result<T> = Result<T, Status_code>;

//Macros
#[macro_export]
macro_rules! save_log {
    ($log_level:expr, $level:expr, $($arg:tt)+) => {
        #[cfg(debug_assertions)]
        if $log_level.is_in(unsafe { $crate::LOG_LEVEL }) {
            ::log::log!($level, "[{:?}]: {}", $log_level, format!($($arg)+));
        }
    };
}

//Enums
#[derive(Debug)]
#[repr(C)]
pub enum Status_code{
    Success = 0,
    Failure = 1,
}

#[cfg(debug_assertions)]
#[derive(Debug, Copy, Clone)]
#[allow(dead_code)]
#[repr(C)]
pub enum Log_level {
    None = 0,
    General = 1,
    Vulkan = 2,
}

//Structs
#[cfg(debug_assertions)]
pub struct Logger{
    buffer: Arc<Mutex<Log_buffer>>,
}

#[cfg(debug_assertions)]
#[derive(Default)]
pub struct Log_buffer {
    size: usize,
    buffers: VecDeque<Vec<u8>>,
    temp_buffer: Vec<u8>,
}

#[cfg(debug_assertions)]
pub struct Log_writer(Arc<Mutex<Log_buffer>>);

//Traits
pub trait C_char_extension<'s> {
    fn to_str(self) -> &'s str;
    fn to_c_str(self) -> &'s CStr;
    fn to_string(self) -> String;
}

//Impls
#[allow(unused_variables)]
impl From<ash::LoadingError> for Status_code{
    fn from(error: ash::LoadingError) -> Self {
        save_log!(Log_level::General, Level::Error, "{:?}", error);
        
        Status_code::Failure
    }
}

#[allow(unused_variables)]
impl From<ash::vk::Result> for Status_code{
    fn from(error: ash::vk::Result) -> Self {
        save_log!(Log_level::General, Level::Error, "{:?}", error);
        
        Status_code::Failure
    }
}

#[cfg(debug_assertions)]
#[allow(dead_code)]
impl Log_level{
    pub fn is_in(&self, other: u64) -> bool{
        (self.clone() as u64 & other) != 0
    }

    pub fn is_empty(&self) -> bool{
        matches!(self, Self::None)
    }
}

#[cfg(debug_assertions)]
impl Logger{
    pub fn new(size: usize) -> Self {
        Logger{
            buffer: Arc::new(Mutex::new(Log_buffer{size, ..Default::default()}))
        }
    }

    pub fn get_writer(&self) -> Log_writer{
        Log_writer(self.buffer.clone())
    }

    pub fn get_last_message_length(&self) -> Option<u64>{
        let log_buffer = self.buffer.lock().unwrap();
        log_buffer.buffers.front().map(|buffer| buffer.len() as u64)
    }

    pub fn get_last_message(&mut self) -> Option<Vec<u8>>{
        let mut log_buffer = self.buffer.lock().unwrap();
        log_buffer.buffers.pop_front()
    }
}

#[cfg(debug_assertions)]
impl Write for Log_writer {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        let mut log_buffer = self.0.lock().unwrap();
        log_buffer.temp_buffer.extend_from_slice(buf);
        Ok(buf.len())
    }

    fn flush(&mut self) -> std::io::Result<()> {
        let mut log_buffer = self.0.lock().unwrap();

        let outdated = log_buffer.buffers.len().saturating_sub(log_buffer.size);
        log_buffer.buffers.drain(0..outdated);

        let new_buffer = std::mem::take(&mut log_buffer.temp_buffer);
        log_buffer.buffers.push_back(new_buffer);
        Ok(())
    }
}

impl<'s> C_char_extension<'s> for *const c_char {
    fn to_str(self) -> &'s str {
        self.to_c_str().to_str().unwrap()
    }
    fn to_c_str(self) -> &'s CStr {
        unsafe { CStr::from_ptr(self) }
    }
    fn to_string(self) -> String {
        self.to_str().to_string()
    }
}