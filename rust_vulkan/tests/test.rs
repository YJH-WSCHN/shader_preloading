//Made by Han_feng

cfg_if::cfg_if! {
    if #[cfg(debug_assertions)] {
        use rust_vulkan::{log_init, logger_init};
        use std::ffi::c_ulonglong;
    }
}

mod test_lib;

#[test]
fn test() {
    #[cfg(debug_assertions)]
    {
        let file = c"log.txt";
        log_init(file.as_ptr(), 3 as c_ulonglong);
    }

    let mut test_application = test_lib::Test_application::default();
    test_application.run();
}

#[cfg(debug_assertions)]
#[test]
fn logger_test(){
    let logger = test_lib::Test_logger::new();
    logger_init(Some(Box::new(logger.logger.get_writer())), 3 as c_ulonglong);
    let _thread = logger.run();

    let mut test_application = test_lib::Test_application::default();
    test_application.run();
}

#[test]
#[ignore]
fn stdout_test(){
    #[cfg(debug_assertions)]
    {
        log_init(std::ptr::null(), 3 as c_ulonglong);
    }

    let mut test_application = test_lib::Test_application::default();
    test_application.run();
}