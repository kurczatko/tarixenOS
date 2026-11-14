/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

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
pub mod r9_280x;
pub mod hd_5850;
pub mod radeon_hd_6870;
pub mod hd_2900_xt;
pub mod rx_590;
