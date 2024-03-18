use std::rc::{Rc,Weak};
use std::cell::RefCell;
use std::ptr::eq;
use std::fmt::{Display,Formatter,Error};

pub enum Orientation {
    V = 0b00,
    H = 0b10,
}
impl Display for Orientation {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(),Error> {
        match self {
            Orientation::V => write!(f, "|"),
            Orientation::H => write!(f, "-"),
        }
    }
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

    fn new_child(&self) -> Option<Rc<RefCell<Region>>> {
        Some(Rc::clone(&self.container
                       .as_ref()?
                       .upgrade()
                       .expect("upgrade failed")
                       .borrow()
                       .subregion()[0]
                       .as_ref()
                       .unwrap()))
    }

    fn inherit_subregions(&self) ->  [Option<Rc<RefCell<Region>>>; 2] {
        match self.from() {
            Some(s) => { 
                match s {
                    Side::L => [self.new_child(),None],
                    Side::R => [None,self.new_child()],
                }
            },
            None => panic!("line 97"), //we are getting here, i am probably too tired for this...
        }
    }

    pub fn split(&mut self, o: Orientation) -> Rc<RefCell<Region>> {
        let new = Rc::new(RefCell::new(Region {
            kind: RegionKind::Split{
                subregion: self.inherit_subregions(),
                fact: 0.5,
                o,
            },
            container: self.container.clone(),
            tags: self.tags,
        }));
        self.container = Some(Rc::downgrade(&new.clone()));
        new
    }
}

impl Display for Region {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(),Error> {
        write!(f, "(")?;
        match &self.kind {
            RegionKind::Client{window} => write!(f, "{}", window),
            RegionKind::Split {subregion,o,..} => { 
                match &subregion[0] {
                    None => write!(f, "{}", o),
                    Some(r) => write!(f, "{}{}", r.borrow(), o),
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
