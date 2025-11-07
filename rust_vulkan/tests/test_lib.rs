//Made by Han_feng

#![allow(non_camel_case_types)]

use raw_window_handle::{HasDisplayHandle, HasWindowHandle};
use winit::application::ApplicationHandler;
use winit::dpi::LogicalSize;
use winit::event::WindowEvent;
use winit::event_loop::ActiveEventLoop;
use winit::platform::windows::EventLoopBuilderExtWindows;
use winit::window::{WindowAttributes, WindowId};
use rust_vulkan::{Vulkan_application, Window};

//Structs
#[derive(Default)]
pub struct Test_application{
    window: Option<winit::window::Window>,
    application: Option<Vulkan_application>,
}

//Impls
impl ApplicationHandler for Test_application {
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        if self.window.is_none() {
            let window = event_loop.create_window(WindowAttributes::default()
                .with_inner_size(LogicalSize::new(800.0, 600.0))
                .with_title("Vulkan test")).unwrap();

            let vulkan_window = Window{
                display_handle: window.display_handle().unwrap().as_raw(),
                window_handle: window.window_handle().unwrap().as_raw(),
                width: window.inner_size().width,
                height: window.inner_size().height,
            };

            self.window = Some(window);
            self.application = Some(Vulkan_application::new(vulkan_window, None::<String>).unwrap());
        }
    }

    fn window_event(&mut self, event_loop: &ActiveEventLoop, _: WindowId, event: WindowEvent) {
        match event {
            WindowEvent::CloseRequested => {
                event_loop.exit();
            }
            WindowEvent::RedrawRequested => {
                self.application.as_mut().unwrap().draw_frame().unwrap();
                self.window.as_mut().unwrap().request_redraw();
            }
            _ => ()
        }
    }
}

impl Test_application{
    pub fn run(&mut self) {
        let mut event_loop_builder = winit::event_loop::EventLoopBuilder::default();
        event_loop_builder.with_any_thread(true);

        let event_loop = event_loop_builder.build().unwrap();
        event_loop.run_app(self).unwrap();
    }
}