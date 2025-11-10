//Made by Han_feng

use std::collections::HashSet;
use crate::libs::utils::{C_char_extension, Status_code, Vulkan_result};
use ash::vk;
use raw_window_handle::{RawDisplayHandle, RawWindowHandle};
use std::ffi::{c_char, c_void, OsStr};
use std::ops::Index;
use std::ptr::NonNull;
use crate::save_log;

cfg_if::cfg_if! {
    if #[cfg(debug_assertions)] {
        use crate::libs::utils::{Log_level};
        use ash::vk::TaggedStructure;
        use log::Level;
    }
}

//Consts
const DEVICE_EXTENSIONS: [*const c_char; 1] = [
    ash::khr::swapchain::NAME.as_ptr()
];

const IMAGE_EXTENSION: u32 = 2;

const VERTEX_SHADER: &[u8] = include_bytes!(concat!(env!("OUT_DIR"), "/shaders/vertex_shader.spv"));
const FRAGMENT_SHADER: &[u8] = include_bytes!(concat!(env!("OUT_DIR"), "/shaders/fragment_shader.spv"));

const MAX_FRAMES_IN_FLIGHT: u32 = 2;

//Structs
#[allow(unused)]
pub struct Vulkan_application {
    //Vulkan attributes
    vulkan_entry: ash::Entry,
    instance: ash::Instance,
    window: Window,
    surface: Surface,
    physical_device: vk::PhysicalDevice,
    indices: Queue_family_indices,
    device: ash::Device,
    queues: Queues,
    swap_chain: Swap_chain,
    graphics_pipelines: Graphics_pipelines,
    command_context: Command_context,
    sync_objects: Sync_objects,

    #[cfg(debug_assertions)]
    _debug_messenger: Debug_messenger,

    //Runtime attributes
    current_frame: usize,
}

#[cfg(debug_assertions)]
struct Debug_messenger {
    instance: ash::ext::debug_utils::Instance,
    messenger: vk::DebugUtilsMessengerEXT,
}

#[derive(Clone, Copy)]
pub struct Window{
    pub display_handle: RawDisplayHandle,
    pub window_handle: RawWindowHandle,
    pub width: u32,
    pub height: u32,
}

struct Surface{
    instance: ash::khr::surface::Instance,
    surface: vk::SurfaceKHR,
}

#[derive(Default, Clone)]
struct Queue_family_indices{
    graphics_family: Option<u32>,
    present_family: Option<u32>,

    //Iterator
    elements: Option<Vec<Option<u32>>>,
}

#[derive(Copy, Clone)]
struct Queues{
    graphics_queue: vk::Queue,
    present_queue: vk::Queue,
}

struct Swap_chain_supports{
    capabilities: vk::SurfaceCapabilitiesKHR,
    formats: Vec<vk::SurfaceFormatKHR>,
    present_modes: Vec<vk::PresentModeKHR>,
}

struct Swap_chain{
    device: ash::khr::swapchain::Device,
    swap_chain: vk::SwapchainKHR,
    render_pass: vk::RenderPass,
    images: Vec<vk::Image>,
    image_views: Vec<vk::ImageView>,
    frame_buffers: Vec<vk::Framebuffer>,

    //Attributes
    surface_format: vk::SurfaceFormatKHR,
    extent: vk::Extent2D,
}

struct Graphics_pipelines {
    layout: vk::PipelineLayout,
    pipelines: Vec<vk::Pipeline>,
}

struct Command_context{
    pool: vk::CommandPool,
    draw_buffers: Vec<vk::CommandBuffer>,
}

struct Sync_objects{
    image_available_semaphores: Vec<vk::Semaphore>,
    render_finished_semaphores: Vec<vk::Semaphore>,
    in_flight_fences: Vec<vk::Fence>,
}

//Impls
impl Vulkan_application {
    pub fn new(window: Window, vulkan_path: Option<impl AsRef<OsStr>>) -> Vulkan_result<Self> {
        let vulkan_entry = Self::get_vulkan_entry(vulkan_path)?;
        let instance = Self::get_instance(&vulkan_entry, window.display_handle)?;

        #[cfg(debug_assertions)]
        let _debug_messenger = Debug_messenger::new(&vulkan_entry, &instance)?;

        let surface = Surface::new(&vulkan_entry, &instance, window)?;

        let (physical_device, indices) = Self::get_physical_device_and_indices(&instance, &surface)?;

        let (device, queues) = Self::get_device_and_queues(&instance, physical_device, &indices)?;

        let swap_chain = Swap_chain::new(&instance, &device, physical_device, &indices, &surface, &window)?;

        let graphics_pipelines = Graphics_pipelines::new(&device, swap_chain.render_pass)?;

        let command_context = Command_context::new(&device, &indices)?;

        let sync_objects = Sync_objects::new(&device, swap_chain.images.len())?;

        save_log!(Log_level::General, Level::Info, "Successfully created vulkan application");
        
        Ok(Vulkan_application{
            vulkan_entry, instance, window, surface, physical_device,
            indices, device, queues, swap_chain, graphics_pipelines,
            command_context, sync_objects,

            #[cfg(debug_assertions)]
            _debug_messenger,

            current_frame: 0,
        })
    }

    pub fn draw_frame(&mut self) -> Vulkan_result<()>{
        unsafe{
            self.device.wait_for_fences(&self.sync_objects.in_flight_fences[self.current_frame..self.current_frame+1], true, u64::MAX)?;
            self.device.reset_fences(&self.sync_objects.in_flight_fences[self.current_frame..self.current_frame+1])?;

            let (image_index, suboptimal) = match self.swap_chain.device.acquire_next_image(self.swap_chain.swap_chain, u64::MAX, self.sync_objects.image_available_semaphores[self.current_frame], vk::Fence::null()){
                Ok((index, suboptimal)) => (index as usize, suboptimal),
                Err(vk::Result::ERROR_OUT_OF_DATE_KHR) => {
                    return Ok(self.swap_chain.recreate(&self.device, self.physical_device, &self.indices, &self.surface, &self.window)?);
                },
                Err(error) => Err(error)?,
            };

            let command_buffers = [
                self.command_context.get_draw_buffer(&self.device, self.current_frame, image_index, &self.swap_chain, &self.graphics_pipelines)?
            ];
            let submit_infos = [
                vk::SubmitInfo::default()
                    .wait_semaphores(&self.sync_objects.image_available_semaphores[self.current_frame..self.current_frame+1])
                    .wait_dst_stage_mask(&[vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT])
                    .command_buffers(&command_buffers)
                    .signal_semaphores(&self.sync_objects.render_finished_semaphores[image_index..image_index+1])
            ];

            self.device.queue_submit(self.queues.graphics_queue, &submit_infos, self.sync_objects.in_flight_fences[self.current_frame])?;

            let swap_chains = [self.swap_chain.swap_chain];
            let image_indices = [image_index as u32];
            let present_info = vk::PresentInfoKHR::default()
                .wait_semaphores(&self.sync_objects.render_finished_semaphores[image_index..image_index+1])
                .swapchains(&swap_chains)
                .image_indices(&image_indices);

            match self.swap_chain.device.queue_present(self.queues.present_queue, &present_info){
                Ok(false) if !suboptimal => (),
                Ok(_) | Err(vk::Result::ERROR_OUT_OF_DATE_KHR) => self.swap_chain.recreate(&self.device, self.physical_device, &self.indices, &self.surface, &self.window)?,
                Err(error) => Err(error)?,
            };
        }

        self.current_frame = (self.current_frame+1) % MAX_FRAMES_IN_FLIGHT as usize;
        Ok(())
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
    
    #[allow(unused_mut)]
    fn get_instance(vulkan_entry: &ash::Entry, display_handle: RawDisplayHandle) -> Vulkan_result<ash::Instance> {
        let app_info = vk::ApplicationInfo::default()
            .application_name(c"Rust Vulkan Application")
            .application_version(vk::make_api_version(0, 1, 0, 0))
            .engine_name(c"Han_feng\'s Engine")
            .engine_version(vk::make_api_version(0, 1, 0, 0))
            .api_version(vk::API_VERSION_1_0);

        let mut enabled_layers = vec![];
        let mut enabled_extensions = ash_window::enumerate_required_extensions(display_handle)?.to_vec();

        cfg_if::cfg_if! {
            if #[cfg(debug_assertions)] {
                enabled_layers.push(c"VK_LAYER_KHRONOS_validation".as_ptr());
                enabled_extensions.push(ash::ext::debug_utils::NAME.as_ptr());
                let mut debug_info = Debug_messenger::populate_debug_info();

                let create_info = vk::InstanceCreateInfo::default()
                .application_info(&app_info)
                .enabled_extension_names(&enabled_extensions)
                .enabled_layer_names(&enabled_layers)
                .push(&mut debug_info);
            }
            else {
                let create_info = vk::InstanceCreateInfo::default()
                .application_info(&app_info)
                .enabled_extension_names(&enabled_extensions)
                .enabled_layer_names(&enabled_layers);
            }
        }

        let instance = unsafe { vulkan_entry.create_instance(&create_info, None)? };

        save_log!(Log_level::General, Level::Info, "Successfully created instance");

        Ok(instance)
    }

    fn get_physical_device_and_indices(instance: &ash::Instance, surface: &Surface) -> Vulkan_result<(vk::PhysicalDevice, Queue_family_indices)>{
        let device_pack = unsafe { instance.enumerate_physical_devices()? .into_iter()
            .filter(|&physical_device| {
                if let Ok(properties) = instance.enumerate_device_extension_properties(physical_device) && let Ok(support) = Swap_chain_supports::physical_device_check(physical_device, &surface) && support {
                    let extensions = properties.into_iter().map(|extension| extension.extension_name.as_ptr().to_string()).collect::<HashSet<_>>();
                    DEVICE_EXTENSIONS.into_iter().all(|extension| extensions.contains(&extension.to_string()))
                }
                else {
                    false
                }
            })
            .filter_map(|physical_device|{
                let properties = instance.get_physical_device_properties(physical_device);
                if let Ok(indices) = Queue_family_indices::new(instance, physical_device, surface){
                    Some((physical_device, indices, properties))
                }
                else {
                    save_log!(Log_level::General, Level::Info, "Device {} skipped", properties.device_name.as_ptr().to_str());
                    None
                }
            })
            .min_by_key(|(_, _, properties)| match properties.device_type {
                vk::PhysicalDeviceType::DISCRETE_GPU => 0,
                vk::PhysicalDeviceType::INTEGRATED_GPU => 1,
                vk::PhysicalDeviceType::VIRTUAL_GPU => 2,
                vk::PhysicalDeviceType::CPU => 3,
                _ => 4
            })
            .ok_or_else(|| {
                save_log!(Log_level::General, Level::Error, "Failed to find suitable physical device");

                Status_code::Failure
            })?};

        save_log!(Log_level::General, Level::Info, "Choose physical device: {}", device_pack.2.device_name.as_ptr().to_str());

        Ok((device_pack.0, device_pack.1))
    }

    fn get_device_and_queues(instance: &ash::Instance, physical_device: vk::PhysicalDevice, indices: &Queue_family_indices) -> Vulkan_result<(ash::Device, Queues)> {
        let queue_infos = indices.clone().filter_map(|index| index).collect::<HashSet<_>>().into_iter().map(|index| {
            vk::DeviceQueueCreateInfo::default()
                .queue_family_index(index)
                .queue_priorities(&[1.0])
        }).collect::<Vec<_>>();

        let device_features = vk::PhysicalDeviceFeatures::default();
        let create_info = vk::DeviceCreateInfo::default()
            .queue_create_infos(&queue_infos)
            .enabled_features(&device_features)
            .enabled_extension_names(&DEVICE_EXTENSIONS);

        let device = unsafe { instance.create_device(physical_device, &create_info, None)? };
        save_log!(Log_level::General, Level::Info, "Successfully created device");
        let queues = Queues::new(&device, &indices);
        save_log!(Log_level::General, Level::Info, "Successfully get queues");

        Ok((device, queues))
    }
}

impl Drop for Vulkan_application {
    fn drop(&mut self) {
        unsafe {
            self.device.device_wait_idle().unwrap();
            
            self.sync_objects.destroy(&self.device);

            self.command_context.destroy(&self.device);

            self.graphics_pipelines.destroy(&self.device);

            self.swap_chain.destroy(&self.device);

            self.surface.destroy();

            #[cfg(debug_assertions)]
            self._debug_messenger.destroy();

            self.device.destroy_device(None);
            self.instance.destroy_instance(None);
        }
    }
}

#[cfg(debug_assertions)]
#[allow(unsafe_op_in_unsafe_fn)]
unsafe extern "system" fn debug_callback(
    message_severity: vk::DebugUtilsMessageSeverityFlagsEXT,
    message_type: vk::DebugUtilsMessageTypeFlagsEXT,
    p_callback_data: *const vk::DebugUtilsMessengerCallbackDataEXT,
    _user_data: *mut c_void) -> vk::Bool32
{
    save_log!(Log_level::Vulkan, match message_severity {
        vk::DebugUtilsMessageSeverityFlagsEXT::ERROR => Level::Error,
        vk::DebugUtilsMessageSeverityFlagsEXT::WARNING => Level::Warn,
        vk::DebugUtilsMessageSeverityFlagsEXT::INFO => Level::Info,
        vk::DebugUtilsMessageSeverityFlagsEXT::VERBOSE => Level::Trace,
        _ => Level::Debug
    }, "({:?}){}", message_type, (*p_callback_data).p_message.to_str());

    vk::FALSE
}

#[cfg(debug_assertions)]
impl Debug_messenger{
    fn new(vulkan_entry: &ash::Entry, instance: &ash::Instance) -> Vulkan_result<Self>{
        let instance = ash::ext::debug_utils::Instance::new(vulkan_entry, instance);
        let messenger = unsafe { instance.create_debug_utils_messenger(&Self::populate_debug_info(), None)? };

        save_log!(Log_level::General, Level::Info, "Successfully created debug messenger");

        Ok(Debug_messenger{
            instance, messenger
        })
    }

    fn populate_debug_info<'debug>() -> vk::DebugUtilsMessengerCreateInfoEXT<'debug>{
        vk::DebugUtilsMessengerCreateInfoEXT::default()
            .message_severity(vk::DebugUtilsMessageSeverityFlagsEXT::ERROR | vk::DebugUtilsMessageSeverityFlagsEXT::WARNING | vk::DebugUtilsMessageSeverityFlagsEXT::VERBOSE)
            .message_type(vk::DebugUtilsMessageTypeFlagsEXT::GENERAL | vk::DebugUtilsMessageTypeFlagsEXT::PERFORMANCE | vk::DebugUtilsMessageTypeFlagsEXT::VALIDATION)
            .pfn_user_callback(Some(debug_callback))
    }

    fn destroy(&self) {
        unsafe {
            self.instance.destroy_debug_utils_messenger(self.messenger, None);
        }
    }
}

impl Window{
    pub fn new(window_handle: *mut c_void, width: u32, height: u32) -> Vulkan_result<Self>{
        Ok(Window{
            display_handle: raw_window_handle::OhosDisplayHandle::new().into(),
            window_handle: raw_window_handle::OhosNdkWindowHandle::new(NonNull::new(window_handle).ok_or_else(|| {
            save_log!(Log_level::General, Level::Error, "Failed to create window handle");
            Status_code::Failure
            })?).into(),
            width, height
        })
    }
}

impl Surface {
    fn new(vulkan_entry: &ash::Entry, instance: &ash::Instance, window: Window) -> Vulkan_result<Self>{
        let surface = unsafe { ash_window::create_surface(vulkan_entry, instance, window.display_handle, window.window_handle, None)? };
        let instance = ash::khr::surface::Instance::new(vulkan_entry, instance);

        save_log!(Log_level::General, Level::Info, "Successfully created surface");

        Ok(Surface{
            surface, instance
        })
    }

    fn destroy(&self) {
        unsafe {
            self.instance.destroy_surface(self.surface, None);
        }
    }
}

impl Queue_family_indices{
    fn new(instance: &ash::Instance, physical_device: vk::PhysicalDevice, surface: &Surface) -> Vulkan_result<Self>{
        let mut indices = Queue_family_indices::default();

        for (index, properties) in unsafe { instance.get_physical_device_queue_family_properties(physical_device) }.iter().enumerate(){
            if properties.queue_flags.contains(vk::QueueFlags::GRAPHICS){
                indices.graphics_family = Some(index as u32);
            }
            if unsafe { surface.instance.get_physical_device_surface_support(physical_device, index as u32, surface.surface)? } {
                indices.present_family = Some(index as u32);
            }

            if indices.is_complete(){
                break;
            }
        }

        if indices.is_complete(){
            Ok(indices)
        }
        else{
            if indices.graphics_family.is_none(){
                save_log!(Log_level::General, Level::Info, "Failed to find graphics queue family indices");
            }
            if indices.present_family.is_none(){
                save_log!(Log_level::General, Level::Info, "Failed to find present queue family indices");
            }

            Err(Status_code::Failure)
        }
    }

    fn is_complete(&self) -> bool{
        self.graphics_family.is_some() && self.present_family.is_some()
    }
}

impl Iterator for Queue_family_indices{
    type Item = Option<u32>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.elements.is_none(){
            self.elements = Some(vec![self.graphics_family, self.present_family]);
        }

        let elements = self.elements.as_mut().unwrap();

        if elements.is_empty(){
            self.elements = None;
            None
        }
        else{
            elements.pop()
        }
    }
}

impl Queues{
    fn new(device: &ash::Device, indices: &Queue_family_indices) -> Self{
        unsafe {
            Queues {
                graphics_queue: device.get_device_queue(indices.graphics_family.unwrap(), 0),
                present_queue: device.get_device_queue(indices.present_family.unwrap(), 0),
            }
        }
    }
}

impl Swap_chain_supports{
    fn new(physical_device: vk::PhysicalDevice, surface: &Surface) -> Vulkan_result<Self>{
        let supports = unsafe {
            Swap_chain_supports{
                capabilities: surface.instance.get_physical_device_surface_capabilities(physical_device, surface.surface)?,
                formats: surface.instance.get_physical_device_surface_formats(physical_device, surface.surface)?,
                present_modes: surface.instance.get_physical_device_surface_present_modes(physical_device, surface.surface)?,
            }
        };

        save_log!(Log_level::General, Level::Info, "Successfully queried swap chain support");

        Ok(supports)
    }

    fn physical_device_check(physical_device: vk::PhysicalDevice, surface: &Surface) -> Vulkan_result<bool>{
        Ok(unsafe {
            !surface.instance.get_physical_device_surface_formats(physical_device, surface.surface)?.is_empty()
            && !surface.instance.get_physical_device_surface_present_modes(physical_device, surface.surface)?.is_empty()
        })
    }

    fn choose_format(&self) -> vk::SurfaceFormatKHR{
        for surface_format in self.formats.iter(){
            if surface_format.format == vk::Format::R8G8B8A8_SRGB && surface_format.color_space == vk::ColorSpaceKHR::SRGB_NONLINEAR{
                return surface_format.clone()
            }
        }

        self.formats[0].clone()
    }

    fn choose_present_mode(&self) -> vk::PresentModeKHR{
        if self.present_modes.contains(&vk::PresentModeKHR::MAILBOX){
            vk::PresentModeKHR::MAILBOX
        }
        else{
            vk::PresentModeKHR::FIFO
        }
    }

    fn choose_extent(&self, width: u32, height: u32) -> vk::Extent2D{
        if self.capabilities.current_extent.width != u32::MAX{
            self.capabilities.current_extent
        }
        else{
            vk::Extent2D{
                width: width.clamp(self.capabilities.min_image_extent.width, self.capabilities.max_image_extent.width),
                height: height.clamp(self.capabilities.min_image_extent.height, self.capabilities.max_image_extent.height)
            }
        }
    }
}

impl Swap_chain{
    fn new(instance: &ash::Instance, device: &ash::Device, physical_device: vk::PhysicalDevice, indices: &Queue_family_indices, surface: &Surface, window: &Window) -> Vulkan_result<Self>{
        let swap_chain_device = ash::khr::swapchain::Device::new(instance, device);
        let (swap_chain, surface_format, extent) = Self::create_swap_chain(&swap_chain_device, physical_device, indices, surface, window)?;

        save_log!(Log_level::General, Level::Info, "Successfully create swap chain");

        let render_pass = Self::create_render_pass(device, surface_format.format)?;

        save_log!(Log_level::General, Level::Info, "Successfully created render pass");

        let mut result = Swap_chain{
            device: swap_chain_device, swap_chain, surface_format, extent, render_pass,
            images: vec![],
            image_views: vec![],
            frame_buffers: vec![]
        };

        result.update_images(device)?;

        Ok(result)
    }

    fn create_swap_chain(device: &ash::khr::swapchain::Device, physical_device: vk::PhysicalDevice, indices: &Queue_family_indices, surface: &Surface, window: &Window) -> Vulkan_result<(vk::SwapchainKHR, vk::SurfaceFormatKHR, vk::Extent2D)> {
        let supports = Swap_chain_supports::new(physical_device, surface)?;

        let surface_format = supports.choose_format();
        let present_mode = supports.choose_present_mode();
        let extent = supports.choose_extent(window.width, window.height);

        let image_count = (supports.capabilities.min_image_count+IMAGE_EXTENSION).min(match supports.capabilities.max_image_count{
            0 => u32::MAX,
            value => value
        });

        let mut create_info = vk::SwapchainCreateInfoKHR::default()
            .surface(surface.surface)
            .min_image_count(image_count)
            .image_format(surface_format.format)
            .image_color_space(surface_format.color_space)
            .image_extent(extent)
            .image_array_layers(1)
            .image_usage(vk::ImageUsageFlags::COLOR_ATTACHMENT);

        let indices_vec = indices.clone().filter_map(|index| index).collect::<Vec<_>>();
        create_info = if indices.present_family != indices.graphics_family{
            create_info.image_sharing_mode(vk::SharingMode::CONCURRENT)
                .queue_family_indices(&indices_vec)
        }
        else{
            create_info.image_sharing_mode(vk::SharingMode::EXCLUSIVE)
        }
            .pre_transform(supports.capabilities.current_transform)
            .composite_alpha(vk::CompositeAlphaFlagsKHR::OPAQUE)
            .present_mode(present_mode)
            .clipped(true);

        Ok((unsafe { device.create_swapchain(&create_info, None) }?, surface_format, extent))
    }

    fn create_render_pass(device: &ash::Device, image_format: vk::Format) -> Vulkan_result<vk::RenderPass> {
        let attachment_descriptions = [
            vk::AttachmentDescription::default()
                .format(image_format)
                .samples(vk::SampleCountFlags::TYPE_1)
                .load_op(vk::AttachmentLoadOp::CLEAR)
                .store_op(vk::AttachmentStoreOp::STORE)
                .stencil_load_op(vk::AttachmentLoadOp::DONT_CARE)
                .stencil_store_op(vk::AttachmentStoreOp::DONT_CARE)
                .initial_layout(vk::ImageLayout::UNDEFINED)
                .final_layout(vk::ImageLayout::PRESENT_SRC_KHR)
        ];

        let attachment_refs = [
            vk::AttachmentReference::default()
                .attachment(0)
                .layout(vk::ImageLayout::COLOR_ATTACHMENT_OPTIMAL)
        ];

        let subpasses = [
            vk::SubpassDescription::default()
                .pipeline_bind_point(vk::PipelineBindPoint::GRAPHICS)
                .color_attachments(&attachment_refs)
        ];

        let dependencies = [
            vk::SubpassDependency::default()
                .src_subpass(vk::SUBPASS_EXTERNAL)
                .dst_subpass(0)
                .src_stage_mask(vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT)
                .src_access_mask(vk::AccessFlags::empty())
                .dst_stage_mask(vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT)
                .dst_access_mask(vk::AccessFlags::COLOR_ATTACHMENT_WRITE)
        ];

        let create_info = vk::RenderPassCreateInfo::default()
            .attachments(&attachment_descriptions)
            .subpasses(&subpasses)
            .dependencies(&dependencies);

        Ok(unsafe { device.create_render_pass(&create_info, None)? })
    }

    fn update_images(&mut self, device: &ash::Device) -> Vulkan_result<()>{
        self.clean_images(device);

        unsafe {
            self.images = self.device.get_swapchain_images(self.swap_chain)?;
            self.image_views = self.images.iter().map(|&image| {
               let create_info = vk::ImageViewCreateInfo::default()
                   .image(image)
                   .view_type(vk::ImageViewType::TYPE_2D)
                   .format(self.surface_format.format)
                   .components(vk::ComponentMapping{
                       r: vk::ComponentSwizzle::IDENTITY,
                       g: vk::ComponentSwizzle::IDENTITY,
                       b: vk::ComponentSwizzle::IDENTITY,
                       a: vk::ComponentSwizzle::IDENTITY,
                   })
                   .subresource_range(vk::ImageSubresourceRange::default()
                       .aspect_mask(vk::ImageAspectFlags::COLOR)
                       .base_mip_level(0)
                       .level_count(1)
                       .base_array_layer(0)
                       .layer_count(1));

                Ok(device.create_image_view(&create_info, None)?)
            }).collect::<Result<_, Status_code>>()?;
            self.frame_buffers = self.image_views.iter().map(|&image_view| {
                let attachments = [image_view];
                let create_info = vk::FramebufferCreateInfo::default()
                    .render_pass(self.render_pass)
                    .attachments(&attachments)
                    .width(self.extent.width)
                    .height(self.extent.height)
                    .layers(1);

                Ok(device.create_framebuffer(&create_info, None)?)
            }).collect::<Result<_, Status_code>>()?;
        }

        save_log!(Log_level::General, Level::Info, "Successfully update swap chain images");

        Ok(())
    }

    fn recreate(&mut self, device: &ash::Device, physical_device: vk::PhysicalDevice, indices: &Queue_family_indices, surface: &Surface, window: &Window) -> Vulkan_result<()>{
        unsafe {
            device.device_wait_idle()?;
            self.device.destroy_swapchain(self.swap_chain, None);
        };

        (self.swap_chain, self.surface_format, self.extent) = Self::create_swap_chain(&self.device, physical_device, indices, surface, window)?;

        self.update_images(device)?;

        save_log!(Log_level::General, Level::Info, "Successfully recreate swap chain");

        Ok(())
    }

    fn clean_images(&self, device: &ash::Device){
        unsafe {
            self.image_views.iter().for_each(|&image_view| device.destroy_image_view(image_view, None));
            self.frame_buffers.iter().for_each(|&frame_buffer| device.destroy_framebuffer(frame_buffer, None));
        }
    }

    fn destroy(&self, device: &ash::Device){
        unsafe {
            self.clean_images(device);
            self.device.destroy_swapchain(self.swap_chain, None);
            device.destroy_render_pass(self.render_pass, None);
        }
    }
}

impl Graphics_pipelines {
    fn new(device: &ash::Device, render_pass: vk::RenderPass) -> Vulkan_result<Self>{
        let vertex_shader = Self::get_shader(device, VERTEX_SHADER)?;
        let fragment_shader = Self::get_shader(device, FRAGMENT_SHADER)?;

        //Shader stages
        let stages = [
            vk::PipelineShaderStageCreateInfo::default()
                .stage(vk::ShaderStageFlags::VERTEX)
                .module(vertex_shader)
                .name(c"main"),
            vk::PipelineShaderStageCreateInfo::default()
                .stage(vk::ShaderStageFlags::FRAGMENT)
                .module(fragment_shader)
                .name(c"main")
        ];

        //Dynamic states
        let dynamic_state_info = vk::PipelineDynamicStateCreateInfo::default()
            .dynamic_states(&[
                vk::DynamicState::VIEWPORT,
                vk::DynamicState::SCISSOR,
            ]);

        //Vertex Input
        let vertex_input_info = vk::PipelineVertexInputStateCreateInfo::default();

        //Input assembly
        let input_assembly_info = vk::PipelineInputAssemblyStateCreateInfo::default()
            .topology(vk::PrimitiveTopology::TRIANGLE_LIST)
            .primitive_restart_enable(false);

        //Viewport
        let viewport_info = vk::PipelineViewportStateCreateInfo::default()
            .viewport_count(1)
            .scissor_count(1);

        //Rasterizer
        let rasterizer_info = vk::PipelineRasterizationStateCreateInfo::default()
            .depth_clamp_enable(false)
            .rasterizer_discard_enable(false)
            .polygon_mode(vk::PolygonMode::FILL)
            .line_width(1.0)
            .cull_mode(vk::CullModeFlags::BACK)
            .front_face(vk::FrontFace::CLOCKWISE)
            .depth_bias_enable(false);

        //Multisample
        let multisample_info = vk::PipelineMultisampleStateCreateInfo::default()
            .sample_shading_enable(false)
            .rasterization_samples(vk::SampleCountFlags::TYPE_1);

        //Depth stencil
        let _depth_stencil_info = vk::PipelineDepthStencilStateCreateInfo::default();

        //Color blend
        let color_blend_attachments = [
            vk::PipelineColorBlendAttachmentState::default()
            .color_write_mask(vk::ColorComponentFlags::RGBA)
            .blend_enable(true)
            .src_color_blend_factor(vk::BlendFactor::SRC_ALPHA)
            .dst_color_blend_factor(vk::BlendFactor::ONE_MINUS_SRC_ALPHA)
            .color_blend_op(vk::BlendOp::ADD)
            .src_alpha_blend_factor(vk::BlendFactor::ONE)
            .dst_alpha_blend_factor(vk::BlendFactor::ZERO)
            .alpha_blend_op(vk::BlendOp::ADD)
        ];

        let color_blend_info = vk::PipelineColorBlendStateCreateInfo::default()
            .logic_op_enable(false)
            .attachments(&color_blend_attachments)
            .blend_constants([0.0, 0.0, 0.0, 0.0]);


        //Layout
        let layout_info = vk::PipelineLayoutCreateInfo::default();

        let layout = unsafe { device.create_pipeline_layout(&layout_info, None)? };

        //Creation
        let create_infos = [
            vk::GraphicsPipelineCreateInfo::default()
            .stages(&stages)
            .vertex_input_state(&vertex_input_info)
            .input_assembly_state(&input_assembly_info)
            .viewport_state(&viewport_info)
            .rasterization_state(&rasterizer_info)
            .multisample_state(&multisample_info)
            .color_blend_state(&color_blend_info)
            .dynamic_state(&dynamic_state_info)
            .layout(layout)
            .render_pass(render_pass)
            .subpass(0)
        ];

        let pipelines = unsafe { device.create_graphics_pipelines(vk::PipelineCache::null(), &create_infos, None).map_err(|(_, err)| err)? };

        save_log!(Log_level::General, Level::Info, "Successfully created graphics pipeline");

        unsafe {
            device.destroy_shader_module(vertex_shader, None);
            device.destroy_shader_module(fragment_shader, None);
        }

        Ok(Graphics_pipelines{
            layout, pipelines
        })
    }

    fn destroy(&self, device: &ash::Device){
        unsafe {
            self.pipelines.iter().for_each(|&pipeline| device.destroy_pipeline(pipeline, None));
            device.destroy_pipeline_layout(self.layout, None);
        }
    }

    fn get_shader(device: &ash::Device, shader_data: &[u8]) -> Vulkan_result<vk::ShaderModule>{
        unsafe {
            let create_info = vk::ShaderModuleCreateInfo::default()
                .code(std::slice::from_raw_parts(shader_data.as_ptr() as *const _, shader_data.len()/4));

            Ok(device.create_shader_module(&create_info, None)?)
        }
    }
}

impl Index<usize> for Graphics_pipelines {
    type Output = vk::Pipeline;

    fn index(&self, index: usize) -> &Self::Output {
        &self.pipelines[index]
    }
}

impl Command_context{
    fn new(device: &ash::Device, indices: &Queue_family_indices) -> Vulkan_result<Self>{
        let pool_info = vk::CommandPoolCreateInfo::default()
            .flags(vk::CommandPoolCreateFlags::RESET_COMMAND_BUFFER)
            .queue_family_index(indices.graphics_family.unwrap());

        let pool = unsafe { device.create_command_pool(&pool_info, None)? };

        let draw_buffers_alloc_info = vk::CommandBufferAllocateInfo::default()
            .command_pool(pool)
            .level(vk::CommandBufferLevel::PRIMARY)
            .command_buffer_count(MAX_FRAMES_IN_FLIGHT);

        let draw_buffers = unsafe { device.allocate_command_buffers(&draw_buffers_alloc_info)? };

        save_log!(Log_level::General, Level::Info, "Successfully created command context");

        Ok(Command_context{
            pool, draw_buffers
        })
    }

    fn get_draw_buffer(&self, device: &ash::Device, index: usize, image_index: usize, swap_chain: &Swap_chain, graphics_pipelines: &Graphics_pipelines) -> Vulkan_result<vk::CommandBuffer>{
        let buffer = self.draw_buffers[index];

        let begin_info = vk::CommandBufferBeginInfo::default();

        let clear_colors = [
            vk::ClearValue{color: vk::ClearColorValue{float32: [0.0, 0.0, 0.0, 0.0]}}
        ];
        let render_pass_info = vk::RenderPassBeginInfo::default()
            .render_pass(swap_chain.render_pass)
            .framebuffer(swap_chain.frame_buffers[image_index])
            .render_area(vk::Rect2D{offset: vk::Offset2D{x: 0, y: 0}, extent: swap_chain.extent})
            .clear_values(&clear_colors);

        let viewports = [
            vk::Viewport::default()
                .x(0.0)
                .y(0.0)
                .width(swap_chain.extent.width as f32)
                .height(swap_chain.extent.height as f32)
                .min_depth(0.0)
                .max_depth(1.0)
        ];
        let scissors = [
            vk::Rect2D::default()
                .offset(vk::Offset2D{x: 0, y: 0})
                .extent(swap_chain.extent)
        ];

        unsafe {
            device.reset_command_buffer(buffer, vk::CommandBufferResetFlags::empty())?;
            device.begin_command_buffer(buffer, &begin_info)?;

            device.cmd_begin_render_pass(buffer, &render_pass_info, vk::SubpassContents::INLINE);
            device.cmd_bind_pipeline(buffer, vk::PipelineBindPoint::GRAPHICS, graphics_pipelines[0]);
            device.cmd_set_viewport(buffer, 0, &viewports);
            device.cmd_set_scissor(buffer, 0, &scissors);
            device.cmd_draw(buffer, 3, 1, 0, 0);
            device.cmd_end_render_pass(buffer);

            device.end_command_buffer(buffer)?;
        }

        Ok(buffer)
    }

    fn destroy(&self, device: &ash::Device){
        unsafe {
            device.destroy_command_pool(self.pool, None);
        }
    }
}

impl Sync_objects {
    fn new(device: &ash::Device, image_count: usize) -> Vulkan_result<Self> {
        let semaphore_info = vk::SemaphoreCreateInfo::default();
        let fence_info = vk::FenceCreateInfo::default()
            .flags(vk::FenceCreateFlags::SIGNALED);

        unsafe {
            Ok(Sync_objects {
                image_available_semaphores: (0..MAX_FRAMES_IN_FLIGHT).map(|_| Ok(device.create_semaphore(&semaphore_info, None)?)).collect::<Result<_, Status_code>>()?,
                render_finished_semaphores: (0..image_count).map(|_| Ok(device.create_semaphore(&semaphore_info, None)?)).collect::<Result<_, Status_code>>()?,
                in_flight_fences: (0..MAX_FRAMES_IN_FLIGHT).map(|_| Ok(device.create_fence(&fence_info, None)?)).collect::<Result<_, Status_code>>()?
            })
        }
    }

    fn destroy(&self, device: &ash::Device){
        unsafe {
            self.image_available_semaphores.iter().for_each(|&s| device.destroy_semaphore(s, None));
            self.render_finished_semaphores.iter().for_each(|&s| device.destroy_semaphore(s, None));
            self.in_flight_fences.iter().for_each(|&f| device.destroy_fence(f, None));
        }
    }
}