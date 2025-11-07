//Made by Han_feng

use std::ffi::{c_char, CStr};

#[cfg(debug_assertions)]
use log::Level;

//Statics
#[cfg(debug_assertions)]
pub static mut LOG_LEVEL: Log_level = Log_level::NONE;

//Types
pub type Vulkan_result<T> = Result<T, Status_code>;

//Macros
#[macro_export]
macro_rules! save_log {
    ($log_level:expr, $level:expr, $($arg:tt)+) => {
        #[cfg(debug_assertions)]
        if unsafe { $crate::LOG_LEVEL }.contains($log_level) {
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

//Structs
#[cfg(debug_assertions)]
#[derive(Copy, Clone, Debug, PartialEq, Eq)]
#[repr(C)]
pub struct Log_level(pub u32);

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
        save_log!(Log_level::GENERAL, Level::Error, "{:?}", error);
        
        Status_code::Failure
    }
}

#[allow(unused_variables)]
impl From<ash::vk::Result> for Status_code{
    fn from(error: ash::vk::Result) -> Self {
        save_log!(Log_level::GENERAL, Level::Error, "{:?}", error);
        
        Status_code::Failure
    }
}

#[cfg(debug_assertions)]
impl Log_level{
    pub const NONE: Log_level = Log_level(0);
    pub const GENERAL: Log_level = Log_level(1 << 0);
    pub const VULKAN: Log_level = Log_level(1 << 1);

    pub fn contains(self, other: Log_level) -> bool{
        (self.0 & other.0) != 0
    }

    pub fn is_empty(self) -> bool{
        self.0 == 0
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