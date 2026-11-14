/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#![no_std]

#[repr(C)]
#[derive(Clone, Copy)]
pub struct CalibrationMatrix {
    pub a: f32,
    pub b: f32,
    pub c: f32,
    pub d: f32,
    pub e: f32,
    pub f: f32,
}

impl CalibrationMatrix {
    pub const fn identity() -> Self {
        CalibrationMatrix {
            a: 1.0,
            b: 0.0,
            c: 0.0,
            d: 0.0,
            e: 1.0,
            f: 0.0,
        }
    }

    pub fn transform(&self, x: i32, y: i32) -> (i32, i32) {
        let fx = x as f32;
        let fy = y as f32;
        let tx = (self.a * fx + self.b * fy + self.c) as i32;
        let ty = (self.d * fx + self.e * fy + self.f) as i32;
        (tx, ty)
    }

    pub fn invert(&self) -> Option<Self> {
        let det = self.a * self.e - self.b * self.d;
        if det.abs() < 1e-6 {
            return None;
        }
        let inv_det = 1.0 / det;
        Some(CalibrationMatrix {
            a: self.e * inv_det,
            b: -self.b * inv_det,
            c: (self.b * self.f - self.c * self.e) * inv_det,
            d: -self.d * inv_det,
            e: self.a * inv_det,
            f: (self.d * self.f - self.a * self.f) * inv_det,
        })
    }
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct CalibrationPoint {
    pub raw_x: i32,
    pub raw_y: i32,
    pub screen_x: i32,
    pub screen_y: i32,
}

impl CalibrationPoint {
    pub const fn new() -> Self {
        CalibrationPoint {
            raw_x: 0,
            raw_y: 0,
            screen_x: 0,
            screen_y: 0,
        }
    }
}

pub const NUM_CALIBRATION_POINTS: usize = 5;

#[repr(C)]
pub struct Calibration {
    pub matrix: CalibrationMatrix,
    pub points: [CalibrationPoint; NUM_CALIBRATION_POINTS],
    pub point_count: u8,
    pub calibrated: u8,
    pub screen_width: u32,
    pub screen_height: u32,
}

impl Calibration {
    pub const fn new() -> Self {
        Calibration {
            matrix: CalibrationMatrix::identity(),
            points: [CalibrationPoint::new(); NUM_CALIBRATION_POINTS],
            point_count: 0,
            calibrated: 0,
            screen_width: 0,
            screen_height: 0,
        }
    }

    pub fn add_point(&mut self, raw_x: i32, raw_y: i32, screen_x: i32, screen_y: i32) -> bool {
        if self.point_count >= NUM_CALIBRATION_POINTS as u8 {
            return false;
        }
        self.points[self.point_count as usize] = CalibrationPoint {
            raw_x,
            raw_y,
            screen_x,
            screen_y,
        };
        self.point_count += 1;
        true
    }

    pub fn compute_matrix(&mut self) -> bool {
        if self.point_count < 3 {
            return false;
        }
        let n = self.point_count as f32;
        let mut sum_x: f32 = 0.0;
        let mut sum_y: f32 = 0.0;
        let mut sum_xx: f32 = 0.0;
        let mut sum_xy: f32 = 0.0;
        let mut sum_yy: f32 = 0.0;
        let mut sum_xsx: f32 = 0.0;
        let mut sum_xsy: f32 = 0.0;
        let mut sum_ysx: f32 = 0.0;
        let mut sum_ysy: f32 = 0.0;
        let i = 0;
        while i < self.point_count as usize {
            let p = self.points[i];
            let rx = p.raw_x as f32;
            let ry = p.raw_y as f32;
            let sx = p.screen_x as f32;
            let sy = p.screen_y as f32;
            sum_x += rx;
            sum_y += ry;
            sum_xx += rx * rx;
            sum_xy += rx * ry;
            sum_yy += ry * ry;
            sum_xsx += rx * sx;
            sum_xsy += rx * sy;
            sum_ysx += ry * sx;
            sum_ysy += ry * sy;
            let i = i + 1;
        }
        let denom = n * (sum_xx * sum_yy - sum_xy * sum_xy);
        if denom.abs() < 1e-6 {
            return false;
        }
        let a = (n * sum_xsx * sum_yy - n * sum_xsy * sum_xy - sum_x * sum_xsx * sum_yy + sum_x * sum_xsy * sum_xy + sum_y * sum_xsy * sum_yy - sum_y * sum_xsy * sum_xy) / denom;
        let b = (n * sum_xsy * sum_xx - n * sum_xsx * sum_xy - sum_x * sum_xsy * sum_xx + sum_x * sum_xsx * sum_xy + sum_y * sum_xsx * sum_xx - sum_y * sum_xsx * sum_xy) / denom;
        let c = (sum_xsx - a * sum_x - b * sum_y) / n;
        let d = (n * sum_ysx * sum_yy - n * sum_ysy * sum_xy - sum_x * sum_ysx * sum_yy + sum_x * sum_ysy * sum_xy + sum_y * sum_ysy * sum_yy - sum_y * sum_ysy * sum_xy) / denom;
        let e = (n * sum_ysy * sum_xx - n * sum_ysx * sum_xy - sum_x * sum_ysy * sum_xx + sum_x * sum_ysx * sum_xy + sum_y * sum_ysx * sum_xx - sum_y * sum_ysx * sum_xy) / denom;
        let f = (sum_ysx - d * sum_x - e * sum_y) / n;
        self.matrix = CalibrationMatrix { a, b, c, d, e, f };
        self.calibrated = 1;
        true
    }

    pub fn map_to_screen(&self, raw_x: i32, raw_y: i32) -> (i32, i32) {
        self.matrix.transform(raw_x, raw_y)
    }

    pub fn reset(&mut self) {
        self.matrix = CalibrationMatrix::identity();
        self.point_count = 0;
        self.calibrated = 0;
    }
}
