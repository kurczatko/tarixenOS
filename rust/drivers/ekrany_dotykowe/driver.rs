#![no_std]

use core::sync::atomic::{AtomicU32, AtomicBool, Ordering};

use crate::calibration::{Calibration, NUM_CALIBRATION_POINTS};
use crate::events::{TouchEvent, TouchEventQueue, TouchPoint, TouchState, MAX_TOUCH_POINTS};

pub const TOUCH_VENDOR_ID: u16 = 0x1234;
pub const TOUCH_DEVICE_ID: u16 = 0x5678;

pub const REG_MMIO_BASE: u32 = 0xF0001000;
pub const REG_IRQ_BASE: u32 = 0x10;

pub const REG_STATUS: u32 = 0x00;
pub const REG_DATA: u32 = 0x04;
pub const REG_CONTROL: u32 = 0x08;
pub const REG_CONFIG: u32 = 0x0C;
pub const REG_INT_ENABLE: u32 = 0x10;
pub const REG_INT_STATUS: u32 = 0x14;
pub const REG_INT_CLEAR: u32 = 0x18;
pub const REG_X_MIN: u32 = 0x20;
pub const REG_X_MAX: u32 = 0x24;
pub const REG_Y_MIN: u32 = 0x28;
pub const REG_Y_MAX: u32 = 0x2C;
pub const REG_WIDTH: u32 = 0x30;
pub const REG_HEIGHT: u32 = 0x34;
pub const REG_PRESSURE_MAX: u32 = 0x38;
pub const REG_FIRMWARE: u32 = 0x3C;
pub const REG_RESET: u32 = 0x40;

pub const CTRL_ENABLE: u32 = 0x00000001;
pub const CTRL_INTERRUPT: u32 = 0x00000002;
pub const CTRL_CALIBRATION_MODE: u32 = 0x00000004;
pub const CTRL_GESTURE_MODE: u32 = 0x00000008;

pub const STATUS_DATA_READY: u32 = 0x00000001;
pub const STATUS_ERROR: u32 = 0x00000002;
pub const STATUS_BUSY: u32 = 0x00000004;

pub const INT_TOUCH: u32 = 0x00000001;
pub const INT_GESTURE: u32 = 0x00000002;
pub const INT_ERROR: u32 = 0x00000004;

pub const MODE_POLLING: u32 = 0;
pub const MODE_INTERRUPT: u32 = 1;

#[repr(C)]
pub struct TouchConfig {
    pub mode: u32,
    pub irq_number: u32,
    pub poll_interval_ms: u32,
    pub screen_width: u32,
    pub screen_height: u32,
    pub enable_gestures: u8,
    pub enable_multi_touch: u8,
    pub touch_threshold: u16,
    pub reserved: [u8; 2],
}

impl TouchConfig {
    pub const fn default() -> Self {
        TouchConfig {
            mode: MODE_POLLING,
            irq_number: REG_IRQ_BASE,
            poll_interval_ms: 10,
            screen_width: 1024,
            screen_height: 768,
            enable_gestures: 1,
            enable_multi_touch: 1,
            touch_threshold: 50,
            reserved: [0; 2],
        }
    }
}

pub struct TouchScreenController {
    mmio_base: *mut u32,
    config: TouchConfig,
    calibration: Calibration,
    state: TouchState,
    queue: TouchEventQueue,
    initialized: AtomicBool,
    total_events: AtomicU32,
    error_count: AtomicU32,
    last_tick: AtomicU32,
}

impl TouchScreenController {
    pub fn new() -> Self {
        TouchScreenController {
            mmio_base: REG_MMIO_BASE as *mut u32,
            config: TouchConfig::default(),
            calibration: Calibration::new(),
            state: TouchState::new(),
            queue: TouchEventQueue::new(),
            initialized: AtomicBool::new(false),
            total_events: AtomicU32::new(0),
            error_count: AtomicU32::new(0),
            last_tick: AtomicU32::new(0),
        }
    }

    pub fn with_config(config: TouchConfig) -> Self {
        TouchScreenController {
            mmio_base: REG_MMIO_BASE as *mut u32,
            config,
            calibration: Calibration::new(),
            state: TouchState::new(),
            queue: TouchEventQueue::new(),
            initialized: AtomicBool::new(false),
            total_events: AtomicU32::new(0),
            error_count: AtomicU32::new(0),
            last_tick: AtomicU32::new(0),
        }
    }

    pub fn detect(&self) -> bool {
        unsafe {
            let vendor = self.read_mmio(0x00);
            let device = self.read_mmio(0x04);
            let rev = self.read_mmio(0x08);
            vendor == TOUCH_VENDOR_ID as u32
                && device == TOUCH_DEVICE_ID as u32
                && rev != 0xFFFFFFFF
        }
    }

    pub fn init(&mut self) -> bool {
        if self.initialized.load(Ordering::SeqCst) {
            return true;
        }
        if !self.detect() {
            return false;
        }
        self.soft_reset();
        self.wait_ready();
        self.write_mmio(REG_CONTROL, CTRL_ENABLE | CTRL_INTERRUPT);
        if self.config.enable_gestures != 0 {
            self.write_mmio(REG_CONTROL, self.read_mmio(REG_CONTROL) | CTRL_GESTURE_MODE);
        }
        if self.config.enable_multi_touch != 0 {
            self.write_mmio(REG_CONTROL, self.read_mmio(REG_CONTROL) | 0x00000010);
        }
        self.write_mmio(REG_INT_ENABLE, INT_TOUCH | INT_GESTURE | INT_ERROR);
        self.write_mmio(REG_X_MIN, 0);
        self.write_mmio(REG_Y_MIN, 0);
        self.write_mmio(REG_WIDTH, self.config.screen_width as u32);
        self.write_mmio(REG_HEIGHT, self.config.screen_height as u32);
        self.write_mmio(REG_PRESSURE_MAX, 255);
        self.calibration.screen_width = self.config.screen_width;
        self.calibration.screen_height = self.config.screen_height;
        self.initialized.store(true, Ordering::SeqCst);
        true
    }

    fn soft_reset(&mut self) {
        unsafe {
            self.write_mmio(REG_RESET, 0x00000001);
        }
        for _ in 0..100000 {
            core::hint::spin_loop();
        }
        unsafe {
            self.write_mmio(REG_RESET, 0x00000000);
        }
    }

    fn wait_ready(&mut self) {
        for _ in 0..1000000 {
            let status = self.read_mmio(REG_STATUS);
            if status & STATUS_BUSY == 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    pub fn shutdown(&mut self) {
        unsafe {
            self.write_mmio(REG_CONTROL, self.read_mmio(REG_CONTROL) & !CTRL_ENABLE);
            self.write_mmio(REG_INT_ENABLE, 0);
        }
        self.initialized.store(false, Ordering::SeqCst);
        self.queue.clear();
        self.state = TouchState::new();
    }

    pub fn is_initialized(&self) -> bool {
        self.initialized.load(Ordering::SeqCst)
    }

    pub fn poll(&mut self) -> bool {
        if !self.initialized.load(Ordering::SeqCst) {
            return false;
        }
        let status = self.read_mmio(REG_STATUS);
        if status & STATUS_DATA_READY == 0 {
            return false;
        }
        let int_status = self.read_mmio(REG_INT_STATUS);
        self.write_mmio(REG_INT_CLEAR, int_status);
        let mut had_event = false;
        if int_status & INT_TOUCH != 0 {
            had_event = self.process_touch_events() || had_event;
        }
        if int_status & INT_GESTURE != 0 {
            had_event = self.process_gesture_event() || had_event;
        }
        if int_status & INT_ERROR != 0 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
        }
        had_event
    }

    pub fn poll_tick(&mut self, tick: u32) -> bool {
        if !self.initialized.load(Ordering::SeqCst) {
            return false;
        }
        let last = self.last_tick.load(Ordering::SeqCst);
        if tick.wrapping_sub(last) < self.config.poll_interval_ms {
            return false;
        }
        self.last_tick.store(tick, Ordering::SeqCst);
        self.poll()
    }

    pub fn interrupt_handler(&mut self) {
        if !self.initialized.load(Ordering::SeqCst) {
            return;
        }
        let int_status = self.read_mmio(REG_INT_STATUS);
        self.write_mmio(REG_INT_CLEAR, int_status);
        if int_status & INT_TOUCH != 0 {
            self.process_touch_events();
        }
        if int_status & INT_GESTURE != 0 {
            self.process_gesture_event();
        }
        if int_status & INT_ERROR != 0 {
            self.error_count.fetch_add(1, Ordering::SeqCst);
        }
    }

    pub fn read_event(&mut self) -> Option<TouchEvent> {
        self.queue.pop()
    }

    pub fn peek_event(&self) -> Option<TouchEvent> {
        self.queue.peek()
    }

    pub fn event_count(&self) -> u32 {
        self.queue.len()
    }

    pub fn clear_events(&mut self) {
        self.queue.clear();
    }

    pub fn set_calibration_point(&mut self, index: usize, raw_x: i32, raw_y: i32, screen_x: i32, screen_y: i32) -> bool {
        if index >= NUM_CALIBRATION_POINTS {
            return false;
        }
        self.calibration.add_point(raw_x, raw_y, screen_x, screen_y);
        true
    }

    pub fn compute_calibration(&mut self) -> bool {
        self.calibration.compute_matrix()
    }

    pub fn is_calibrated(&self) -> bool {
        self.calibration.calibrated != 0
    }

    pub fn reset_calibration(&mut self) {
        self.calibration.reset();
    }

    pub fn map_raw_to_screen(&self, raw_x: i32, raw_y: i32) -> (i32, i32) {
        self.calibration.map_to_screen(raw_x, raw_y)
    }

    pub fn get_state(&self) -> TouchState {
        self.state
    }

    pub fn total_events(&self) -> u32 {
        self.total_events.load(Ordering::SeqCst)
    }

    pub fn error_count(&self) -> u32 {
        self.error_count.load(Ordering::SeqCst)
    }

    pub fn screen_size(&self) -> (u32, u32) {
        (self.config.screen_width, self.config.screen_height)
    }

    pub fn set_screen_size(&mut self, width: u32, height: u32) {
        self.config.screen_width = width;
        self.config.screen_height = height;
        self.calibration.screen_width = width;
        self.calibration.screen_height = height;
        self.write_mmio(REG_WIDTH, width);
        self.write_mmio(REG_HEIGHT, height);
    }

    fn process_touch_events(&mut self) -> bool {
        let mut had_event = false;
        let raw_count = self.read_mmio(REG_DATA) & 0xFF;
        let mut i = 0;
        while i < raw_count.min(MAX_TOUCH_POINTS as u32) {
            let base = i * 8;
            let touch_id = (self.read_mmio(REG_DATA + 4 + base) & 0xFF) as u8;
            let raw_x = self.read_mmio(REG_DATA + 8 + base) as i32;
            let raw_y = self.read_mmio(REG_DATA + 12 + base) as i32;
            let pressure = (self.read_mmio(REG_DATA + 16 + base) & 0xFF) as u8;
            let (screen_x, screen_y) = self.calibration.map_to_screen(raw_x, raw_y);
            let width = (self.read_mmio(REG_DATA + 20 + base) & 0xFFFF) as u16;
            let height = (self.read_mmio(REG_DATA + 24 + base) & 0xFFFF) as u16;
            let active = if pressure > self.config.touch_threshold { 1u8 } else { 0 };
            let slot = self.state.find_point(touch_id);
            let event_type = match (slot, active) {
                (None, 1) => TouchEventType::Press,
                (Some(_), 0) => TouchEventType::Release,
                (Some(_), 1) => TouchEventType::Move,
                (None, 0) => TouchEventType::Move,
            };
            if let Some(slot_idx) = slot {
                if active == 0 {
                    self.state.points[slot_idx].active = 0;
                    self.state.active_count = self.state.active_count.saturating_sub(1);
                } else {
                    self.state.points[slot_idx] = TouchPoint {
                        touch_id, x: screen_x, y: screen_y, raw_x, raw_y,
                        pressure, width, height, active, reserved: [0; 3],
                    };
                }
            } else if active == 1 {
                if let Some(free) = self.state.find_free_slot() {
                    self.state.points[free] = TouchPoint {
                        touch_id, x: screen_x, y: screen_y, raw_x, raw_y,
                        pressure, width, height, active, reserved: [0; 3],
                    };
                    self.state.active_count += 1;
                }
            }
            let event = TouchEvent {
                event_type: event_type as u8,
                touch_id,
                x: screen_x,
                y: screen_y,
                raw_x,
                raw_y,
                pressure,
                width,
                height,
                timestamp: self.total_events.load(Ordering::SeqCst) as u64,
                gesture_id: 0,
                reserved: [0; 3],
            };
            self.queue.push(event);
            self.total_events.fetch_add(1, Ordering::SeqCst);
            had_event = true;
            i += 1;
        }
        had_event
    }

    fn process_gesture_event(&mut self) -> bool {
        let gesture_id = (self.read_mmio(REG_DATA + 0x40) & 0xFF) as u8;
        let x = self.read_mmio(REG_DATA + 0x44) as i32;
        let y = self.read_mmio(REG_DATA + 0x48) as i32;
        let event = TouchEvent {
            event_type: TouchEventType::Gesture as u8,
            touch_id: 0,
            x,
            y,
            raw_x: x,
            raw_y: y,
            pressure: 0,
            width: 0,
            height: 0,
            timestamp: self.total_events.load(Ordering::SeqCst) as u64,
            gesture_id,
            reserved: [0; 3],
        };
        self.queue.push(event);
        self.total_events.fetch_add(1, Ordering::SeqCst);
        true
    }

    fn read_mmio(&self, offset: u32) -> u32 {
        unsafe { self.mmio_base.add(offset as usize / 4).read_volatile() }
    }

    fn write_mmio(&mut self, offset: u32, value: u32) {
        unsafe { self.mmio_base.add(offset as usize / 4).write_volatile(value); }
    }
}

static mut CTRL_INSTANCE: Option<TouchScreenController> = None;

fn ctrl_instance() -> &'static mut TouchScreenController {
    unsafe {
        if CTRL_INSTANCE.is_none() {
            CTRL_INSTANCE = Some(TouchScreenController::new());
        }
        CTRL_INSTANCE.as_mut().unwrap()
    }
}

pub fn touch_driver_init() -> bool {
    let ctrl = ctrl_instance();
    ctrl.init()
}

pub fn touch_driver_shutdown() {
    let ctrl = ctrl_instance();
    ctrl.shutdown();
}

pub fn touch_driver_detected() -> bool {
    let ctrl = ctrl_instance();
    ctrl.detect()
}

pub fn touch_driver_is_initialized() -> bool {
    let ctrl = ctrl_instance();
    ctrl.is_initialized()
}

pub fn touch_driver_poll(tick: u32) -> bool {
    let ctrl = ctrl_instance();
    ctrl.poll_tick(tick)
}

pub fn touch_driver_interrupt() {
    let ctrl = ctrl_instance();
    ctrl.interrupt_handler();
}

pub fn touch_driver_read_event() -> Option<TouchEvent> {
    let ctrl = ctrl_instance();
    ctrl.read_event()
}

pub fn touch_driver_peek_event() -> Option<TouchEvent> {
    let ctrl = ctrl_instance();
    ctrl.peek_event()
}

pub fn touch_driver_event_count() -> u32 {
    let ctrl = ctrl_instance();
    ctrl.event_count()
}

pub fn touch_driver_clear_events() {
    let ctrl = ctrl_instance();
    ctrl.clear_events();
}

pub fn touch_driver_set_calibration_point(index: usize, raw_x: i32, raw_y: i32, screen_x: i32, screen_y: i32) -> bool {
    let ctrl = ctrl_instance();
    ctrl.set_calibration_point(index, raw_x, raw_y, screen_x, screen_y)
}

pub fn touch_driver_compute_calibration() -> bool {
    let ctrl = ctrl_instance();
    ctrl.compute_calibration()
}

pub fn touch_driver_is_calibrated() -> bool {
    let ctrl = ctrl_instance();
    ctrl.is_calibrated()
}

pub fn touch_driver_reset_calibration() {
    let ctrl = ctrl_instance();
    ctrl.reset_calibration();
}

pub fn touch_driver_map_to_screen(raw_x: i32, raw_y: i32) -> (i32, i32) {
    let ctrl = ctrl_instance();
    ctrl.map_raw_to_screen(raw_x, raw_y)
}

pub fn touch_driver_total_events() -> u32 {
    let ctrl = ctrl_instance();
    ctrl.total_events()
}

pub fn touch_driver_error_count() -> u32 {
    let ctrl = ctrl_instance();
    ctrl.error_count()
}
