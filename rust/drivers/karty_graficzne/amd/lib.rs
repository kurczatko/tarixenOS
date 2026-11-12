#![no_std]
#![no_main]

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

pub mod r7_265;
pub mod rx_580;
pub mod hd_4730;
pub mod rx_460; 
pub mod hd_5850;
pub mod radeon_hd_6870;
pub mod hd_2900_xt;
pub mod rx_590;
