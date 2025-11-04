//Made by Han_feng

use crate::{save_log, Log_level, Status_code};
use ash::vk;
use std::ffi::{CStr, OsStr};
use std::os::raw::c_void;
use log::Level;
use crate::libs::utils::Vulkan_result;

#[cfg(debug_assertions)]
unsafe extern "system" fn debug_callback(
    message_severity: vk::DebugUtilsMessageSeverityFlagsEXT,
    message_type: vk::DebugUtilsMessageTypeFlagsEXT,
    p_callback_data: *const vk::DebugUtilsMessengerCallbackDataEXT,
    _user_data: *mut c_void,
) -> vk::Bool32 {
    let message = CStr::from_ptr((*p_callback_data).p_message).to_str().unwrap();
    match message_severity {
        vk::DebugUtilsMessageSeverityFlagsEXT::VERBOSE => {
            save_log!(Log_level::Vulkan, Level::Debug, "[Vulkan][{:?}][{:?}]: {}", message_severity, message_type, message);
        }
        vk::DebugUtilsMessageSeverityFlagsEXT::INFO => {
            save_log!(Log_level::Vulkan, Level::Info, "[Vulkan][{:?}][{:?}]: {}", message_severity, message_type, message);
        }
        vk::DebugUtilsMessageSeverityFlagsEXT::WARNING => {
            save_log!(Log_level::Vulkan, Level::Warn, "[Vulkan][{:?}][{:?}]: {}", message_severity, message_type, message);
        }
        vk::DebugUtilsMessageSeverityFlagsEXT::ERROR => {
            save_log!(Log_level::Vulkan, Level::Error, "[Vulkan][{:?}][{:?}]: {}", message_severity, message_type, message);
        }
        _ => ()
    };

    vk::FALSE
}

//Structs
pub struct Vulkan_application {
    vulkan_entry: ash::Entry,
    instance: ash::Instance,

    #[cfg(debug_assertions)] _debug_messenger: Debug_messenger,
}

#[cfg(debug_assertions)]
struct Debug_messenger{
    instance: ash::ext::debug_utils::Instance,
    messenger: vk::DebugUtilsMessengerEXT,
}

struct Queue_family_indices{
    graphics_family: Option<u32>,
    present_family: Option<u32>,
}

//Impls
impl Vulkan_application{
    pub fn new(vulkan_path: Option<impl AsRef<OsStr>>) -> Vulkan_result<Vulkan_application> {
        let vulkan_entry = Self::get_vulkan_entry(vulkan_path)?;
        let instance = Self::get_instance(&vulkan_entry)?;

        #[cfg(debug_assertions)]
        let _debug_messenger = Debug_messenger::new(&vulkan_entry, &instance)?;

        Ok(Vulkan_application{
            vulkan_entry, instance,

            #[cfg(debug_assertions)] _debug_messenger
        })
    }

    fn get_vulkan_entry(vulkan_path: Option<impl AsRef<OsStr>>) -> Vulkan_result<ash::Entry> {
        Ok(unsafe {
            if let Some(path) = vulkan_path {
                ash::Entry::load_from(path)?
            }
            else {
                ash::Entry::load()?
            }
        })
    }

    fn get_instance(vulkan_entry: &ash::Entry) -> Vulkan_result<ash::Instance> {
        let app_info = vk::ApplicationInfo::default()
            .application_name(c"Rust vulkan application")
            .application_version(vk::make_api_version(0, 1, 0, 0))
            .engine_name(c"Han_feng\'s Engine")
            .engine_version(vk::make_api_version(0, 1, 0, 0))
            .api_version(vk::make_api_version(0, 1, 0, 0));

        cfg_if::cfg_if!(
            if #[cfg(debug_assertions)] {
                let mut debug_info = Debug_messenger::populate_create_info();
                let create_info = vk::InstanceCreateInfo::default()
                    .application_info(&app_info)
                    .push_next(&mut debug_info);
            }
            else {
                let create_info = vk::InstanceCreateInfo::default()
                    .application_info(&app_info);
            }
        );

        let instance = unsafe {
            vulkan_entry.create_instance(&create_info, None)?
        };

        save_log!(Log_level::General, Level::Info, "[General][application] Successfully created vulkan instance");

        Ok(instance)
    }
}

impl Drop for Vulkan_application {
    fn drop(&mut self) {
        unsafe {
            #[cfg(debug_assertions)]
            self._debug_messenger.destroy();

            self.instance.destroy_instance(None);
        }
    }
}

#[cfg(debug_assertions)]
impl Debug_messenger{
    fn new(vulkan_entry: &ash::Entry, instance: &ash::Instance) -> Vulkan_result<Debug_messenger> {
        let instance = ash::ext::debug_utils::Instance::new(vulkan_entry, instance);

        let create_info = Self::populate_create_info();
        let messenger = unsafe {
            instance.create_debug_utils_messenger(&create_info, None)?
        };

        save_log!(Log_level::General, Level::Info, "[General][application] Successfully created Debug messenger");

        Ok(Debug_messenger{
            instance, messenger
        })
    }

    fn destroy(&self) {
        unsafe {
            self.instance.destroy_debug_utils_messenger(self.messenger, None);
        }
    }
}

#[cfg(debug_assertions)]
impl<'debug> Debug_messenger{
    fn populate_create_info() -> vk::DebugUtilsMessengerCreateInfoEXT<'debug>{
        vk::DebugUtilsMessengerCreateInfoEXT::default()
            .message_severity(vk::DebugUtilsMessageSeverityFlagsEXT::ERROR | vk::DebugUtilsMessageSeverityFlagsEXT::WARNING | vk::DebugUtilsMessageSeverityFlagsEXT::VERBOSE)
            .message_type(vk::DebugUtilsMessageTypeFlagsEXT::GENERAL | vk::DebugUtilsMessageTypeFlagsEXT::PERFORMANCE | vk::DebugUtilsMessageTypeFlagsEXT::VALIDATION)
            .pfn_user_callback(Some(debug_callback))
    }
}

impl Queue_family_indices {
    fn new() -> Queue_family_indices {
        todo!()
    }
}