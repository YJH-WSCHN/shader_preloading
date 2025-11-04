//Made by Han_feng

#[cfg(debug_assertions)]
use log::Level;
use std::ops::BitAnd;
use std::sync::atomic::AtomicU32;

pub type Vulkan_result<T> = Result<T, Status_code>;

//Macro rules
#[cfg(debug_assertions)]
#[macro_export]
macro_rules! save_log {
    ($log_level:expr, $level:expr, $format:expr, $($object:expr), *) => {
        #[cfg(debug_assertions)]
        if $crate::LOG_LEVEL.load(::std::sync::atomic::Ordering::Relaxed) & $log_level {
            ::log::log!($level, $format, $($object), *);
        }
    };
    ($log_level:expr, $level:expr, $message:expr) => {
        #[cfg(debug_assertions)]
        if $crate::LOG_LEVEL.load(::std::sync::atomic::Ordering::Relaxed) & $log_level {
            ::log::log!($level, $message);
        }
    }
}

//Status Code
#[repr(C)]
pub enum Status_code {
    Success = 0,
    Failure = 1,
}

impl From<ash::LoadingError> for Status_code {
    fn from(error: ash::LoadingError) -> Status_code {
        save_log!(Log_level::General, Level::Error, "[General][Ash] Loading Error: {:?}", error);

        Status_code::Failure
    }
}

impl From<ash::vk::Result> for Status_code {
    fn from(error: ash::vk::Result) -> Status_code {
        save_log!(Log_level::General, Level::Error, "[General][Ash] Vulkan Result Error: {:?}", error);

        Status_code::Failure
    }
}

//Log level
#[cfg(debug_assertions)]
pub static LOG_LEVEL: AtomicU32 = AtomicU32::new(0);

#[cfg(debug_assertions)]
#[repr(C)]
pub enum Log_level{
    None = 0,
    General = 1,
    Vulkan = 2,
}

impl From<u32> for Log_level{
    fn from(level: u32) -> Self {
        match level {
            0 => Log_level::None,
            1 => Log_level::General,
            2 => Log_level::Vulkan,
            _ => Log_level::None,
        }
    }
}

impl BitAnd<Log_level> for u32{
    type Output = bool;

    fn bitand(self, rhs: Log_level) -> Self::Output {
        self & rhs as u32 != 0
    }
}
