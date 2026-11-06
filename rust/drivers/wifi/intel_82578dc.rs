#![no_std]

use core::sync::atomic::{AtomicU32, Ordering};

pub const INTEL_VENDOR_ID: u16 = 0x8086;
pub const INTEL_82578DC_DEVICE_ID: u16 = 0x1502;

pub const REG_MMIO_BASE: u32 = 0xF0000000;
pub const REG_IO_BASE: u32 = 0xE000;

pub const CTRL: u32 = 0x0000;
pub const STATUS: u32 = 0x0008;
pub const CTRL_EXT: u32 = 0x0018;
pub const ICWR: u32 = 0x1700;
pub const CUSTOM: u32 = 0x1704;
pub const LEDCTL: u32 = 0xE000;
pub const FCAH: u32 = 0x5420;
pub const FCAL: u32 = 0x5400;
pub const FCTTV: u32 = 0x1700;
pub const TXCW: u32 = 0x3428;
pub const RXCW: u32 = 0x3400;
pub const RCTL: u32 = 0x0100;
pub const TCTL: u32 = 0x0400;
pub const RDH: u32 = 0x2810;
pub const RDT: u32 = 0x2818;
pub const TDH: u32 = 0x3810;
pub const TDT: u32 = 0x3818;
pub const RDBAH: u32 = 0x2814;
pub const RDBAL: u32 = 0x2800;
pub const TDBAL: u32 = 0x3800;
pub const TDBAH: u32 = 0x3804;
pub const RLEN: u32 = 0x2810;
pub const TLEN: u32 = 0x3808;
pub const IMR: u32 = 0x0D08;
pub const ICR: u32 = 0x0C00;
pub const EICS: u32 = 0x0C20;
pub const EIMS: u32 = 0x0C80;
pub const EIMC: u32 = 0x0C88;
pub const EIAC: u32 = 0x0C8C;
pub const EIAM: u32 = 0x0C84;
pub const RXTCTL: u32 = 0x3C00;
pub const RXDCTL: u32 = 0x2828;
pub const TXDCTL: u32 = 0x3828;
pub const RAL0: u32 = 0x5400;
pub const RAH0: u32 = 0x5404;
pub const RDBAL_desc: u32 = 0x0000;
pub const RDBAH_desc: u32 = 0x0008;
pub const RDLEN_desc: u32 = 0x0010;
pub const RDH_desc: u32 = 0x0018;
pub const RDT_desc: u32 = 0x001C;

static IS_INITIALIZED: AtomicU32 = AtomicU32::new(0);
static MAC_ADDRESS: [u8; 6] = [0x00, 0x1B, 0x21, 0xAA, 0xBB, 0xCC];
static LINK_UP: AtomicU32 = AtomicU32::new(0);

static mut TX_DESC_BASE: *mut u32 = core::ptr::null_mut();
static mut RX_DESC_BASE: *mut u32 = core::ptr::null_mut();
static mut TX_DESC_HEAD: u32 = 0;
static mut TX_DESC_TAIL: u32 = 0;
static mut RX_DESC_HEAD: u32 = 0;
static mut RX_DESC_TAIL: u32 = 0;
static mut TX_DESC_COUNT: u32 = 256;
static mut RX_DESC_COUNT: u32 = 256;

static mut net_send_fn: Option<unsafe extern "C" fn(*const u8, u32) -> i32> = None;
static mut net_receive_fn: Option<unsafe extern "C" fn(*mut u8, u32) -> i32> = None;

#[repr(C)]
pub struct MacAddress {
    pub addr: [u8; 6],
}

#[repr(C)]
pub struct WifiStats {
    pub packets_sent: u32,
    pub packets_received: u32,
    pub bytes_sent: u32,
    pub bytes_received: u32,
    pub errors: u32,
}

static mut STATS: WifiStats = WifiStats {
    packets_sent: 0,
    packets_received: 0,
    bytes_sent: 0,
    bytes_received: 0,
    errors: 0,
};

pub struct Intel82578Dc {
    mmio_base: *mut u32,
    io_base: u16,
}

impl Intel82578Dc {
    pub fn new() -> Self {
        Intel82578Dc {
            mmio_base: REG_MMIO_BASE as *mut u32,
            io_base: REG_IO_BASE as u16,
        }
    }

    pub fn detect(&self) -> bool {
        unsafe {
            let vendor = self.read_mmio(0x00);
            let device = self.read_mmio(0x04);
            vendor == INTEL_VENDOR_ID as u32 && device == INTEL_82578DC_DEVICE_ID as u32
        }
    }

    pub fn init(&mut self) -> bool {
        if IS_INITIALIZED.load(Ordering::SeqCst) == 1 {
            return true;
        }
        if !self.detect() {
            return false;
        }
        self.reset();
        self.wait_idle();
        self.setup_mac_address();
        self.setup_rx_descriptors();
        self.setup_tx_descriptors();
        self.setup_interrupts();
        self.enable_receiver();
        self.enable_transmitter();
        self.set_link_up();
        IS_INITIALIZED.store(1, Ordering::SeqCst);
        true
    }

    fn reset(&mut self) {
        self.write_mmio(CTRL, self.read_mmio(CTRL) | 0x00000004);
        for _ in 0..50000 {
            core::hint::spin_loop();
        }
    }

    fn wait_idle(&mut self) {
        for _ in 0..1000000 {
            let status = self.read_mmio(STATUS);
            if status & 0x00000400 != 0 {
                return;
            }
            core::hint::spin_loop();
        }
    }

    fn setup_mac_address(&mut self) {
        let mut mac_lo: u32 = 0;
        let mut mac_hi: u32 = 0;
        for i in 0..4 {
            mac_lo |= (MAC_ADDRESS[i] as u32) << (i * 8);
        }
        for i in 4..6 {
            mac_hi |= (MAC_ADDRESS[i] as u32) << ((i - 4) * 8);
        }
        self.write_mmio(RAL0, mac_lo);
        self.write_mmio(RAH0, mac_hi | 0x80000000);
    }

    fn setup_rx_descriptors(&mut self) {
        self.write_mmio(RDBAL_desc, 0x00010000);
        self.write_mmio(RDBAH_desc, 0x00000000);
        self.write_mmio(RDLEN_desc, unsafe { RX_DESC_COUNT } * 16);
        self.write_mmio(RDH_desc, 0x00000000);
        self.write_mmio(RDT_desc, unsafe { RX_DESC_COUNT } - 1);
    }

    fn setup_tx_descriptors(&mut self) {
        self.write_mmio(TDBAL, 0x00020000);
        self.write_mmio(TDBAH, 0x00000000);
        self.write_mmio(TLEN, unsafe { TX_DESC_COUNT } * 16);
        self.write_mmio(TDH, 0x00000000);
        self.write_mmio(TDT, 0x00000000);
    }

    fn setup_interrupts(&mut self) {
        self.write_mmio(IMR, 0x00000001);
        self.write_mmio(EIMS, 0x00000001);
        self.write_mmio(EIAC, 0x00000001);
    }

    fn enable_receiver(&mut self) {
        self.write_mmio(RCTL, 0x00000001 | 0x00000002 | 0x00000004);
    }

    fn enable_transmitter(&mut self) {
        self.write_mmio(TCTL, 0x00000001 | 0x00000002);
    }

    fn set_link_up(&mut self) {
        LINK_UP.store(1, Ordering::SeqCst);
    }

    fn read_mmio(&self, offset: u32) -> u32 {
        unsafe { self.mmio_base.add(offset as usize / 4).read_volatile() }
    }

    fn write_mmio(&mut self, offset: u32, value: u32) {
        unsafe { self.mmio_base.add(offset as usize / 4).write_volatile(value); }
    }

    pub fn send_packet(&mut self, data: &[u8]) -> bool {
        if !LINK_UP.load(Ordering::SeqCst) == 1 {
            return false;
        }
        let tail = unsafe { TX_DESC_TAIL };
        let head = unsafe { TX_DESC_HEAD };
        if (tail + 1) % unsafe { TX_DESC_COUNT } == head {
            return false;
        }
        unsafe {
            let desc_addr = TX_DESC_BASE.add((tail * 16) as usize);
            core::ptr::write_volatile(desc_addr, data.as_ptr() as u32);
            core::ptr::write_volatile(desc_addr.add(1), data.len() as u32);
            core::ptr::write_volatile(desc_addr.add(2), 0x01000000);
            core::ptr::write_volatile(desc_addr.add(3), 0x00000000);
        }
        unsafe { TX_DESC_TAIL = (tail + 1) % unsafe { TX_DESC_COUNT }; }
        self.write_mmio(TDT, unsafe { TX_DESC_TAIL });
        unsafe { STATS.packets_sent += 1; STATS.bytes_sent += data.len() as u32; }
        true
    }

    pub fn receive_packet(&mut self, buffer: &mut [u8]) -> Option<usize> {
        if !LINK_UP.load(Ordering::SeqCst) == 1 {
            return None;
        }
        let head = unsafe { RX_DESC_HEAD };
        let tail = unsafe { RX_DESC_TAIL };
        if head == tail {
            return None;
        }
        unsafe {
            let desc_addr = RX_DESC_BASE.add((head * 16) as usize);
            let len = core::ptr::read_volatile(desc_addr.add(1)) as usize;
            if len == 0 || len > buffer.len() {
                return None;
            }
            let data_ptr = core::ptr::read_volatile(desc_addr) as *const u8;
            for i in 0..len {
                buffer[i] = core::ptr::read_volatile(data_ptr.add(i));
            }
            core::ptr::write_volatile(desc_addr.add(1), 0);
            RX_DESC_HEAD = (head + 1) % unsafe { RX_DESC_COUNT };
            self.write_mmio(RDT, (head + unsafe { RX_DESC_COUNT } - 1) % unsafe { RX_DESC_COUNT });
            STATS.packets_received += 1;
            STATS.bytes_received += len as u32;
            Some(len)
        }
    }

    pub fn set_callbacks(&mut self, send_fn: unsafe extern "C" fn(*const u8, u32) -> i32, receive_fn: unsafe extern "C" fn(*mut u8, u32) -> i32) {
        unsafe {
            net_send_fn = Some(send_fn);
            net_receive_fn = Some(receive_fn);
        }
    }

    pub fn link_up(&self) -> bool {
        LINK_UP.load(Ordering::SeqCst) == 1
    }

    pub fn stats(&self) -> WifiStats {
        unsafe { core::ptr::read(&STATS) }
    }

    pub fn mac_address(&self) -> MacAddress {
        MacAddress { addr: MAC_ADDRESS }
    }

    pub fn tx_packets(&self) -> u32 {
        unsafe { STATS.packets_sent }
    }

    pub fn rx_packets(&self) -> u32 {
        unsafe { STATS.packets_received }
    }

    pub fn tx_bytes(&self) -> u32 {
        unsafe { STATS.bytes_sent }
    }

    pub fn rx_bytes(&self) -> u32 {
        unsafe { STATS.bytes_received }
    }

    pub fn errors(&self) -> u32 {
        unsafe { STATS.errors }
    }
}

static mut GPU_INSTANCE: Option<Intel82578Dc> = None;

pub fn gpu_instance() -> &'static mut Intel82578Dc {
    unsafe {
        if GPU_INSTANCE.is_none() {
            GPU_INSTANCE = Some(Intel82578Dc::new());
        }
        GPU_INSTANCE.as_mut().unwrap()
    }
}

pub fn intel_82578dc_init() -> bool {
    let wifi = gpu_instance();
    wifi.init()
}

pub fn intel_82578dc_detected() -> bool {
    let wifi = gpu_instance();
    wifi.detect()
}

#[no_mangle]
pub extern "C" fn intel_82578dc_driver_init() -> bool {
    intel_82578dc_init()
}

#[no_mangle]
pub extern "C" fn intel_82578dc_driver_detected() -> bool {
    intel_82578dc_detected()
}

#[no_mangle]
pub extern "C" fn intel_82578dc_send_packet(data: *const u8, len: u32) -> bool {
    let wifi = gpu_instance();
    unsafe { wifi.send_packet(core::slice::from_raw_parts(data, len as usize)) }
}

#[no_mangle]
pub extern "C" fn intel_82578dc_receive_packet(buffer: *mut u8, len: u32) -> i32 {
    let wifi = gpu_instance();
    unsafe { wifi.receive_packet(core::slice::from_raw_parts_mut(buffer, len as usize)).map(|l| l as i32).unwrap_or(-1) }
}

#[no_mangle]
pub extern "C" fn intel_82578dc_link_up() -> bool {
    let wifi = gpu_instance();
    wifi.link_up()
}

#[no_mangle]
pub extern "C" fn intel_82578dc_stats_packets_sent() -> u32 {
    let wifi = gpu_instance();
    wifi.tx_packets()
}

#[no_mangle]
pub extern "C" fn intel_82578dc_stats_packets_received() -> u32 {
    let wifi = gpu_instance();
    wifi.rx_packets()
}

#[no_mangle]
pub extern "C" fn intel_82578dc_stats_bytes_sent() -> u32 {
    let wifi = gpu_instance();
    wifi.tx_bytes()
}

#[no_mangle]
pub extern "C" fn intel_82578dc_stats_bytes_received() -> u32 {
    let wifi = gpu_instance();
    wifi.rx_bytes()
}

#[no_mangle]
pub extern "C" fn intel_82578dc_stats_errors() -> u32 {
    let wifi = gpu_instance();
    wifi.errors()
}

#[no_mangle]
pub extern "C" fn intel_82578dc_set_callbacks(send_fn: unsafe extern "C" fn(*const u8, u32) -> i32, receive_fn: unsafe extern "C" fn(*mut u8, u32) -> i32) {
    let wifi = gpu_instance();
    wifi.set_callbacks(send_fn, receive_fn);
}

#[no_mangle]
pub extern "C" fn intel_82578dc_mac_address(buffer: *mut u8) {
    let wifi = gpu_instance();
    let mac = wifi.mac_address();
    unsafe {
        for i in 0..6 {
            core::ptr::write(buffer.add(i), mac.addr[i]);
        }
    }
}
