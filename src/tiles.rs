use std::rc::{Rc,Weak};
use std::cell::RefCell;
use std::ptr::eq;
use std::fmt::{Debug,Formatter,Error,Display};

#[derive(Debug)]
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

impl Side {
    #[inline]
    pub fn other(&self) -> Side {
        match self {//unsafe approach with mem::transmute is same
                    //but replaces sete with xor instruction
            Side::L => Side::R,
            Side::R => Side::L,
        }
    }
}

pub enum Direction {
    N = Orientation::H as isize | Side::L as isize, //0b10
    S = Orientation::H as isize | Side::R as isize, //0b11
    E = Orientation::V as isize | Side::L as isize, //0b00
    W = Orientation::V as isize | Side::R as isize, //0b01
}

#[derive(Debug)]
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

#[derive(Debug)]
pub struct Region {//SHOULDNT BE PUB
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
    fn subregion(&self) -> &[Option<Rc<RefCell<Region>>>; 2] {
        if let RegionKind::Split{subregion,..} = &self.kind {
            subregion
        } else {
            panic!("subregion() should not be called on client nodes.");
        }
    }

    pub fn from(&self) -> Option<Side> {
        self.container
            .as_ref()? //the root node is the ONLY node st `from() == None`
            .upgrade()
            .unwrap()
            .borrow()
            .subregion()
            .iter()
            .map_while(|sub| sub.as_ref())
            .zip([Side::L, Side::R])
            .find_map(|(sub,side)| eq(sub.as_ptr(),self).then_some(side))
    }

    pub fn adopt(&mut self, child: Rc<RefCell<Region>>, s: Side) -> Option<Rc<RefCell<Region>>> {
        if let RegionKind::Split{subregion,..} = &mut self.kind {
            subregion[s as usize].replace(child)
        } else {
            panic!("only splits can adopt nodes")
        }
    }

    fn swap_in(old: &Rc<RefCell<Region>>, new: Rc<RefCell<Region>>) -> Option<Rc<RefCell<Region>>> {
        old.borrow()
            .container
            .as_ref()?
            .upgrade()
            .unwrap()
            .borrow_mut()
            .adopt(new, old.borrow().from().unwrap_or(Side::L))
    }

    pub fn new_split(to_split: Rc<RefCell<Region>>, o: Orientation) -> Rc<RefCell<Region>> {
        let mut new_s: Option<Weak<RefCell<Region>>> = None;
        let split = Rc::new_cyclic(|split| { 
            new_s = Some(split.clone());
            RefCell::new(Region {
                kind: RegionKind::Split{
                    subregion: [Some(to_split.clone()), None],
                    fact: 0.5,
                    o,
                },
                container: to_split.borrow().container.clone(),
                tags: to_split.borrow().tags,
            })
        }
        );
        Region::swap_in(&to_split, split.clone());
        to_split.borrow_mut().container = new_s;
        split
    }
}

impl Display for Region {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(),Error> {
        write!(f, "(")?;
        match &self.kind {
            RegionKind::Client{window} => write!(f, "{}", window),
            RegionKind::Split {subregion,o,..} => { 
                match &subregion[0] {
                    None => write!(f, "{:?}", o),
                    Some(r) => write!(f, "{}{:?}", r.borrow(), o),
                }.and_then(|_| { 
                    let mut e = Ok(());
                    subregion[1].as_ref().inspect(|r| { e = write!(f, "{}", r.borrow()); });
                    e
                })
            },
        }?;
        write!(f, ")")
    }
}
