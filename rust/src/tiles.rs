use std::rc::{Rc,Weak};
use std::ptr::eq;

enum Orientation {
    V = 0b00,
    H = 0b10,
}

enum Side {
    L = 0b00,
    R = 0b01,
}

enum Direction {
    North = Orientation::H as isize | Side::L as isize,
    South = Orientation::H as isize | Side::R as isize,
    East = Orientation::V as isize | Side::L as isize,
    West = Orientation::V as isize | Side::R as isize,
}

enum RegionKind {
    Split { 
        subregion: [Option<Rc<Region>>; 2],
        fact: f32,
        o: Orientation,
    },
    Client{ 
        window: i32,
    },
}

struct Region {
    kind: RegionKind,
    container: Option<Rc<Region>>,
    tags: u8,
}

pub struct Tiling {
    whole: Option<Rc<Region>>,
    curr: Option<Rc<Region>>,
    filter: u8,
}

impl Region {
    fn from(&self) -> Option<Side> {
        if let RegionKind::Split{subregion,..} = &self.container.as_ref()?.kind {
            match &subregion[0] {
                None => None,
                Some(r) => if eq(Rc::<Region>::as_ptr(&r),self) {
                    Some(Side::L)
                } else {
                    None
                }.or_else(|| { match &subregion[1] {
                    None => None,
                    Some(r) => if eq(Rc::<Region>::as_ptr(&r),self) {
                        Some(Side::R)
                    } else {
                        None
                    }
                }})
            }
        } else {
            None
        }
    }
}
