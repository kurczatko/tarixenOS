#![no_std]

use core::slice;

mod events;
mod calibration;
mod driver;

use crate::driver::{TouchScreenController, TouchConfig, TouchEvent};

pub use driver::{
    touch_driver_init,
    touch_driver_shutdown,
    touch_driver_detected,
    touch_driver_is_initialized,
    touch_driver_poll,
    touch_driver_interrupt,
    touch_driver_read_event,
    touch_driver_peek_event,
    touch_driver_event_count,
    touch_driver_clear_events,
    touch_driver_set_calibration_point,
    touch_driver_compute_calibration,
    touch_driver_is_calibrated,
    touch_driver_reset_calibration,
    touch_driver_map_to_screen,
    touch_driver_total_events,
    touch_driver_error_count,
};

pub use driver::{TouchScreenController, TouchConfig};

pub fn touch_driver_register_config(
    mode: u32,
    irq_number: u32,
    poll_interval_ms: u32,
    screen_width: u32,
    screen_height: u32,
    enable_gestures: u8,
    enable_multi_touch: u8,
    touch_threshold: u16,
) {
    unsafe {
        let ctrl = driver::ctrl_instance();
        ctrl.config = TouchConfig {
            mode,
            irq_number,
            poll_interval_ms,
            screen_width,
            screen_height,
            enable_gestures,
            enable_multi_touch,
            touch_threshold,
            reserved: [0; 2],
        };
    }
}

#[repr(C)]
pub struct TouchDriverApi {
    pub init: unsafe extern "C" fn() -> bool,
    pub shutdown: unsafe extern "C" fn(),
    pub detected: unsafe extern "C" fn() -> bool,
    pub is_initialized: unsafe extern "C" fn() -> bool,
    pub poll: unsafe extern "C" fn(u32) -> bool,
    pub interrupt: unsafe extern "C" fn(),
    pub read_event: unsafe extern "C" fn(*mut TouchEvent) -> bool,
    pub peek_event: unsafe extern "C" fn(*mut TouchEvent) -> bool,
    pub event_count: unsafe extern "C" fn() -> u32,
    pub clear_events: unsafe extern "C" fn(),
    pub set_calibration_point: unsafe extern "C" fn(usize, i32, i32, i32, i32) -> bool,
    pub compute_calibration: unsafe extern "C" fn() -> bool,
    pub is_calibrated: unsafe extern "C" fn() -> bool,
    pub reset_calibration: unsafe extern "C" fn(),
    pub map_to_screen: unsafe extern "C" fn(i32, i32, *mut i32, *mut i32),
    pub total_events: unsafe extern "C" fn() -> u32,
    pub error_count: unsafe extern "C" fn() -> u32,
}

pub static TOUCH_DRIVER_API: TouchDriverApi = TouchDriverApi {
    init: touch_driver_init_c,
    shutdown: touch_driver_shutdown_c,
    detected: touch_driver_detected_c,
    is_initialized: touch_driver_is_initialized_c,
    poll: touch_driver_poll_c,
    interrupt: touch_driver_interrupt_c,
    read_event: touch_driver_read_event_c,
    peek_event: touch_driver_peek_event_c,
    event_count: touch_driver_event_count_c,
    clear_events: touch_driver_clear_events_c,
    set_calibration_point: touch_driver_set_calibration_point_c,
    compute_calibration: touch_driver_compute_calibration_c,
    is_calibrated: touch_driver_is_calibrated_c,
    reset_calibration: touch_driver_reset_calibration_c,
    map_to_screen: touch_driver_map_to_screen_c,
    total_events: touch_driver_total_events_c,
    error_count: touch_driver_error_count_c,
};

#[no_mangle]
pub extern "C" fn touch_driver_api() -> *const TouchDriverApi {
    &TOUCH_DRIVER_API as *const TouchDriverApi
}

#[no_mangle]
pub extern "C" fn touch_driver_init_c() -> bool {
    touch_driver_init()
}

#[no_mangle]
pub extern "C" fn touch_driver_shutdown_c() {
    touch_driver_shutdown();
}

#[no_mangle]
pub extern "C" fn touch_driver_detected_c() -> bool {
    touch_driver_detected()
}

#[no_mangle]
pub extern "C" fn touch_driver_is_initialized_c() -> bool {
    touch_driver_is_initialized()
}

#[no_mangle]
pub extern "C" fn touch_driver_poll_c(tick: u32) -> bool {
    touch_driver_poll(tick)
}

#[no_mangle]
pub extern "C" fn touch_driver_interrupt_c() {
    touch_driver_interrupt();
}

#[no_mangle]
pub extern "C" fn touch_driver_read_event_c(out: *mut TouchEvent) -> bool {
    if out.is_null() {
        return false;
    }
    match touch_driver_read_event() {
        Some(event) => {
            unsafe { out.write(event); }
            true
        }
        None => false,
    }
}

#[no_mangle]
pub extern "C" fn touch_driver_peek_event_c(out: *mut TouchEvent) -> bool {
    if out.is_null() {
        return false;
    }
    match touch_driver_peek_event() {
        Some(event) => {
            unsafe { out.write(event); }
            true
        }
        None => false,
    }
}

#[no_mangle]
pub extern "C" fn touch_driver_event_count_c() -> u32 {
    touch_driver_event_count()
}

#[no_mangle]
pub extern "C" fn touch_driver_clear_events_c() {
    touch_driver_clear_events();
}

#[no_mangle]
pub extern "C" fn touch_driver_set_calibration_point_c(index: usize, raw_x: i32, raw_y: i32, screen_x: i32, screen_y: i32) -> bool {
    touch_driver_set_calibration_point(index, raw_x, raw_y, screen_x, screen_y)
}

#[no_mangle]
pub extern "C" fn touch_driver_compute_calibration_c() -> bool {
    touch_driver_compute_calibration()
}

#[no_mangle]
pub extern "C" fn touch_driver_is_calibrated_c() -> bool {
    touch_driver_is_calibrated()
}

#[no_mangle]
pub extern "C" fn touch_driver_reset_calibration_c() {
    touch_driver_reset_calibration();
}

#[no_mangle]
pub extern "C" fn touch_driver_map_to_screen_c(raw_x: i32, raw_y: i32, out_x: *mut i32, out_y: *mut i32) {
    if out_x.is_null() || out_y.is_null() {
        return;
    }
    let (x, y) = touch_driver_map_to_screen(raw_x, raw_y);
    unsafe {
        out_x.write(x);
        out_y.write(y);
    }
}

#[no_mangle]
pub extern "C" fn touch_driver_total_events_c() -> u32 {
    touch_driver_total_events()
}

#[no_mangle]
pub extern "C" fn touch_driver_error_count_c() -> u32 {
    touch_driver_error_count()
}
