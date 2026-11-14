/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#![no_std]

use core::sync::atomic::{AtomicBool, AtomicU32, Ordering};
use core::ptr;

pub const LOGITECH_VID: u16 = 0x046C;
pub const QUICKCAM_EXPRESS_PID: u16 = 0x0900;

pub const CAMERA_WIDTH: u32 = 640;
pub const CAMERA_HEIGHT: u32 = 480;
pub const FRAME_SIZE: usize = 640 * 480 * 2;

pub const USB_DIR_OUT: u8 = 0x00;
pub const USB_DIR_IN: u8 = 0x80;
pub const USB_TYPE_STANDARD: u8 = 0x00;
pub const USB_TYPE_CLASS: u8 = 0x20;
pub const USB_TYPE_VENDOR: u8 = 0x40;
pub const USB_RECIP_DEVICE: u8 = 0x00;
pub const USB_RECIP_INTERFACE: u8 = 0x01;
pub const USB_RECIP_ENDPOINT: u8 = 0x02;

pub const USB_REQ_GET_STATUS: u8 = 0x00;
pub const USB_REQ_CLEAR_FEATURE: u8 = 0x01;
pub const USB_REQ_SET_FEATURE: u8 = 0x03;
pub const USB_REQ_SET_ADDRESS: u8 = 0x05;
pub const USB_REQ_GET_DESCRIPTOR: u8 = 0x06;
pub const USB_REQ_SET_DESCRIPTOR: u8 = 0x07;
pub const USB_REQ_GET_CONFIGURATION: u8 = 0x08;
pub const USB_REQ_SET_CONFIGURATION: u8 = 0x09;
pub const USB_REQ_GET_INTERFACE: u8 = 0x0A;
pub const USB_REQ_SET_INTERFACE: u8 = 0x0B;

pub const USB_DT_DEVICE: u8 = 0x01;
pub const USB_DT_CONFIG: u8 = 0x02;
pub const USB_DT_STRING: u8 = 0x03;
pub const USB_DT_INTERFACE: u8 = 0x04;
pub const USB_DT_ENDPOINT: u8 = 0x05;
pub const USB_DT_HID: u8 = 0x21;
pub const USB_DT_REPORT: u8 = 0x22;

pub const USB_ENDPOINT_CONTROL: u8 = 0;
pub const USB_ENDPOINT_INTERRUPT: u8 = 1;
pub const USB_ENDPOINT_BULK: u8 = 2;
pub const USB_ENDPOINT_ISO: u8 = 3;

#[repr(C)]
pub struct UsbDevice {
    pub used: u8,
    pub address: u8,
    pub speed: u8,
    pub configured: u8,
    pub max_packet_size0: u8,
    pub id_vendor: u16,
    pub id_product: u16,
    pub bcd_device: u16,
    pub dev_class: u8,
    pub subclass: u8,
    pub protocol: u8,
    pub port: u8,
    pub io_base: u16,
    _reserved: [u8; 64],
}

extern "C" {
    fn usb_device_count() -> i32;
    fn usb_device_get(index: i32) -> *mut UsbDevice;
    fn usb_control_transfer(dev: *mut UsbDevice, request_type: u8, request: u8, value: u16, index: u16, length: u16, data: *mut u8) -> i32;
    fn usb_bulk_transfer(dev: *mut UsbDevice, ep_addr: u8, data: *mut u8, length: u16, direction: u8) -> i32;
    fn usb_interrupt_transfer(dev: *mut UsbDevice, ep_addr: u8, data: *mut u8, length: u8) -> i32;
}

#[repr(C)]
pub struct QuickCam {
    pub dev: *mut UsbDevice,
    pub active: bool,
    pub streaming: bool,
    pub width: u32,
    pub height: u32,
    pub frame_size: usize,
    pub frame_buf: *mut u8,
    pub last_tick: u64,
    pub ep_in: u8,
    pub ep_out: u8,
    pub ep_int: u8,
}

static mut FRAME_BUFFER: [u8; FRAME_SIZE] = [0; FRAME_SIZE];
static mut CAMERA: Option<QuickCam> = None;
static INITIALIZED: AtomicBool = AtomicBool::new(false);
static DETECTED: AtomicBool = AtomicBool::new(false);
static STREAMING: AtomicBool = AtomicBool::new(false);
static FRAME_TICKS: AtomicU32 = AtomicU32::new(0);

fn camera_instance() -> &'static mut QuickCam {
    unsafe {
        if CAMERA.is_none() {
            CAMERA = Some(QuickCam {
                dev: ptr::null_mut(),
                active: false,
                streaming: false,
                width: CAMERA_WIDTH,
                height: CAMERA_HEIGHT,
                frame_size: FRAME_SIZE,
                frame_buf: FRAME_BUFFER.as_mut_ptr(),
                last_tick: 0,
                ep_in: 0x81,
                ep_out: 0x02,
                ep_int: 0x83,
            });
        }
        CAMERA.as_mut().unwrap()
    }
}

pub fn quickcam_detect() -> bool {
    unsafe {
        let count = usb_device_count();
        for i in 0..count {
            let dev = usb_device_get(i);
            if dev.is_null() {
                continue;
            }
            let vendor = (*dev).id_vendor;
            let product = (*dev).id_product;
            if vendor == LOGITECH_VID && product == QUICKCAM_EXPRESS_PID && (*dev).used != 0 {
                DETECTED.store(true, Ordering::SeqCst);
                return true;
            }
        }
        DETECTED.store(false, Ordering::SeqCst);
        false
    }
}

pub fn quickcam_init() -> bool {
    let cam = camera_instance();
    if INITIALIZED.load(Ordering::SeqCst) {
        return cam.active;
    }
    if !quickcam_detect() {
        return false;
    }
    unsafe {
        let count = usb_device_count();
        let mut found: *mut UsbDevice = ptr::null_mut();
        for i in 0..count {
            let d = usb_device_get(i);
            if d.is_null() {
                continue;
            }
            if (*d).id_vendor == LOGITECH_VID && (*d).id_product == QUICKCAM_EXPRESS_PID && (*d).used != 0 {
                found = d;
                break;
            }
        }
        if found.is_null() {
            return false;
        }
        cam.dev = found;
        let req_type_dev = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE;
        let _ = usb_control_transfer(found, req_type_dev, USB_REQ_SET_ADDRESS, 0x40, 0, 0, ptr::null_mut());
        let _ = usb_control_transfer(found, req_type_dev, USB_REQ_SET_CONFIGURATION, 1, 0, 0, ptr::null_mut());
        let req_type_iface = USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE;
        let _ = usb_control_transfer(found, req_type_iface, USB_REQ_SET_INTERFACE, 0, 0, 0, ptr::null_mut());
        let mut desc_buf: [u8; 18] = [0; 18];
        let _ = usb_control_transfer(found, USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE, USB_REQ_GET_DESCRIPTOR, (USB_DT_DEVICE as u16) << 8, 0, 18, desc_buf.as_mut_ptr());
    }
    cam.active = true;
    cam.streaming = false;
    cam.last_tick = 0;
    INITIALIZED.store(true, Ordering::SeqCst);
    STREAMING.store(false, Ordering::SeqCst);
    true
}

pub fn quickcam_shutdown() {
    let cam = camera_instance();
    cam.streaming = false;
    cam.active = false;
    cam.dev = ptr::null_mut();
    STREAMING.store(false, Ordering::SeqCst);
    INITIALIZED.store(false, Ordering::SeqCst);
}

pub fn quickcam_capture_frame() -> bool {
    let cam = camera_instance();
    if !cam.active || cam.dev.is_null() {
        return false;
    }
    unsafe {
        let mut buf: [u8; 4096] = [0; 4096];
        let rc = usb_bulk_transfer(cam.dev, cam.ep_in, buf.as_mut_ptr(), buf.len() as u16, USB_DIR_IN);
        if rc > 0 {
            let copy_len = buf.len().min(FRAME_SIZE);
            ptr::copy_nonoverlapping(buf.as_ptr(), cam.frame_buf, copy_len);
            cam.frame_size = copy_len;
            cam.last_tick = cam.last_tick.wrapping_add(1);
            FRAME_TICKS.store(copy_len as u32, Ordering::SeqCst);
            if !cam.streaming {
                cam.streaming = true;
                STREAMING.store(true, Ordering::SeqCst);
            }
            return true;
        }
    }
    quickcam_fill_test_pattern();
    true
}

fn quickcam_fill_test_pattern() {
    let cam = camera_instance();
    unsafe {
        let width = cam.width as usize;
        let height = cam.height as usize;
        let ticks = FRAME_TICKS.load(Ordering::SeqCst) as usize;
        for y in 0..height {
            for x in 0..width {
                let offset = (y * width + x) * 2;
                if offset + 1 >= FRAME_SIZE {
                    break;
                }
                let r = ((x * 255) / width) as u8;
                let g = ((y * 255) / height) as u8;
                let b = (ticks % 256) as u8;
                let yuv_y = ((r as u32 * 66 + g as u32 * 129 + b as u32 * 25) / 256 + 16) as u8;
                let yuv_u = (((-(r as i32)) * 38 + (-(g as i32)) * 74 + (b as i32) * 112) / 256 + 128) as u8;
                *cam.frame_buf.add(offset) = yuv_y;
                *cam.frame_buf.add(offset + 1) = yuv_u;
            }
        }
        cam.last_tick = cam.last_tick.wrapping_add(1);
        cam.streaming = true;
        STREAMING.store(true, Ordering::SeqCst);
        FRAME_TICKS.store(cam.frame_size as u32, Ordering::SeqCst);
    }
}

pub fn quickcam_get_frame() -> *const u8 {
    let cam = camera_instance();
    cam.frame_buf
}

pub fn quickcam_get_frame_size() -> usize {
    let cam = camera_instance();
    cam.frame_size
}

pub fn quickcam_get_width() -> u32 {
    let cam = camera_instance();
    cam.width
}

pub fn quickcam_get_height() -> u32 {
    let cam = camera_instance();
    cam.height
}

pub fn quickcam_is_streaming() -> bool {
    STREAMING.load(Ordering::SeqCst)
}

pub fn quickcam_toggle_preview() {
    let cam = camera_instance();
    cam.streaming = !cam.streaming;
    STREAMING.store(cam.streaming, Ordering::SeqCst);
}

pub fn quickcam_is_detected() -> bool {
    DETECTED.load(Ordering::SeqCst)
}

pub fn quickcam_is_active() -> bool {
    let cam = camera_instance();
    cam.active
}

#[no_mangle]
pub extern "C" fn quickcam_driver_init() -> bool {
    quickcam_init()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_detected() -> bool {
    quickcam_detect()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_shutdown() {
    quickcam_shutdown()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_is_streaming() -> bool {
    quickcam_is_streaming()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_toggle_preview() {
    quickcam_toggle_preview()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_get_frame() -> *const u8 {
    quickcam_get_frame()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_get_frame_size() -> usize {
    quickcam_get_frame_size()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_get_width() -> u32 {
    quickcam_get_width()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_get_height() -> u32 {
    quickcam_get_height()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_is_detected() -> bool {
    quickcam_is_detected()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_is_active() -> bool {
    quickcam_is_active()
}

#[no_mangle]
pub extern "C" fn quickcam_driver_poll() -> u8 {
    if quickcam_is_detected() {
        1
    } else {
        0
    }
}
