/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#![no_std]

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum TouchEventType {
    Press = 0,
    Release = 1,
    Move = 2,
    Gesture = 3,
}

#[repr(C)]
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum GestureType {
    None = 0,
    Tap = 1,
    DoubleTap = 2,
    Swipe = 3,
    PinchZoom = 4,
    Rotate = 5,
    LongPress = 6,
    EdgeSwipeLeft = 7,
    EdgeSwipeRight = 8,
    EdgeSwipeUp = 9,
    EdgeSwipeDown = 10,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct TouchPoint {
    pub touch_id: u8,
    pub x: i32,
    pub y: i32,
    pub raw_x: i32,
    pub raw_y: i32,
    pub pressure: u8,
    pub width: u16,
    pub height: u16,
    pub active: u8,
    pub reserved: [u8; 3],
}

impl TouchPoint {
    pub const fn new() -> Self {
        TouchPoint {
            touch_id: 0,
            x: 0,
            y: 0,
            raw_x: 0,
            raw_y: 0,
            pressure: 0,
            width: 0,
            height: 0,
            active: 0,
            reserved: [0; 3],
        }
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct TouchEvent {
    pub event_type: u8,
    pub touch_id: u8,
    pub x: i32,
    pub y: i32,
    pub raw_x: i32,
    pub raw_y: i32,
    pub pressure: u8,
    pub width: u16,
    pub height: u16,
    pub timestamp: u64,
    pub gesture_id: u8,
    pub reserved: [u8; 3],
}

impl TouchEvent {
    pub const fn new() -> Self {
        TouchEvent {
            event_type: TouchEventType::Press as u8,
            touch_id: 0,
            x: 0,
            y: 0,
            raw_x: 0,
            raw_y: 0,
            pressure: 0,
            width: 0,
            height: 0,
            timestamp: 0,
            gesture_id: GestureType::None as u8,
            reserved: [0; 3],
        }
    }

    pub fn is_press(&self) -> bool {
        self.event_type == TouchEventType::Press as u8
    }

    pub fn is_release(&self) -> bool {
        self.event_type == TouchEventType::Release as u8
    }

    pub fn is_move(&self) -> bool {
        self.event_type == TouchEventType::Move as u8
    }

    pub fn is_gesture(&self) -> bool {
        self.event_type == TouchEventType::Gesture as u8
    }
}

pub const MAX_TOUCH_POINTS: usize = 10;
pub const EVENT_QUEUE_SIZE: usize = 64;

#[repr(C)]
pub struct TouchEventQueue {
    pub buffer: [TouchEvent; EVENT_QUEUE_SIZE],
    pub head: u32,
    pub tail: u32,
    pub count: u32,
}

impl TouchEventQueue {
    pub const fn new() -> Self {
        TouchEventQueue {
            buffer: [TouchEvent::new(); EVENT_QUEUE_SIZE],
            head: 0,
            tail: 0,
            count: 0,
        }
    }

    pub fn push(&mut self, event: TouchEvent) -> bool {
        if self.count >= EVENT_QUEUE_SIZE as u32 {
            return false;
        }
        self.buffer[self.tail as usize] = event;
        self.tail = (self.tail + 1) % EVENT_QUEUE_SIZE as u32;
        self.count += 1;
        true
    }

    pub fn pop(&mut self) -> Option<TouchEvent> {
        if self.count == 0 {
            return None;
        }
        let event = self.buffer[self.head as usize];
        self.head = (self.head + 1) % EVENT_QUEUE_SIZE as u32;
        self.count -= 1;
        Some(event)
    }

    pub fn peek(&self) -> Option<TouchEvent> {
        if self.count == 0 {
            return None;
        }
        Some(self.buffer[self.head as usize])
    }

    pub fn len(&self) -> u32 {
        self.count
    }

    pub fn is_empty(&self) -> bool {
        self.count == 0
    }

    pub fn clear(&mut self) {
        self.head = 0;
        self.tail = 0;
        self.count = 0;
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct TouchState {
    pub points: [TouchPoint; MAX_TOUCH_POINTS],
    pub active_count: u8,
    pub last_timestamp: u64,
}

impl TouchState {
    pub const fn new() -> Self {
        TouchState {
            points: [TouchPoint::new(); MAX_TOUCH_POINTS],
            active_count: 0,
            last_timestamp: 0,
        }
    }

    pub fn find_point(&self, touch_id: u8) -> Option<usize> {
        for i in 0..MAX_TOUCH_POINTS {
            if self.points[i].active != 0 && self.points[i].touch_id == touch_id {
                return Some(i);
            }
        }
        None
    }

    pub fn find_free_slot(&self) -> Option<usize> {
        for i in 0..MAX_TOUCH_POINTS {
            if self.points[i].active == 0 {
                return Some(i);
            }
        }
        None
    }

    pub fn active_points(&self) -> impl Iterator<Item = &TouchPoint> {
        self.points.iter().filter(|p| p.active != 0)
    }
}
