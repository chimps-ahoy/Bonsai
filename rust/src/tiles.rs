use std::rc::{Rc,Weak};
use std::cell::RefCell;
use std::ptr::eq;

pub enum Orientation {
    V = 0b00,
    H = 0b10,
}

#[derive(Debug)]
#[derive(PartialEq)]
pub enum Side {
    L = 0b00,
    R = 0b01,
}

pub enum Direction {
    North = Orientation::H as isize | Side::L as isize,
    South = Orientation::H as isize | Side::R as isize,
    East = Orientation::V as isize | Side::L as isize,
    West = Orientation::V as isize | Side::R as isize,
}

pub enum RegionKind {//SHOULDNT BE PUB
    Split { 
        subregion: [Option<Rc<RefCell<Region>>>; 2],
        fact: f32,
        o: Orientation,
    },
    Client{ 
        window: i32,
    },
}

pub struct Region {//SHOULDNT BE PUB...?
    pub kind: RegionKind,
    pub container: Option<Weak<RefCell<Region>>>,
    pub tags: u8,
}

pub struct Tiling {
    whole: Option<Rc<Region>>,
    curr: Option<Rc<Region>>,
    filter: u8,
}

impl Region {

    fn subregion(&self) -> Option<&[Option<Rc<RefCell<Region>>>; 2]> {
        if let RegionKind::Split{subregion,..} = &self.kind {
            Some(subregion)
        } else {
            None
        }
    }

    pub fn from(&self) -> Option<Side> {
        self.container
            .as_ref()? //the root node is the ONLY node st `from() == None`
            .upgrade()
            .unwrap()
            .borrow()
            .subregion()
            .unwrap()
            .iter()
            .map_while(|sub| sub.as_ref())
            .zip([Side::L, Side::R])
            .find_map(|(sub,side)| eq(sub.as_ptr(),self).then_some(side))
    }
}
