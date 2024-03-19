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

pub enum Direction {
    North = Orientation::H as isize | Side::L as isize,
    South = Orientation::H as isize | Side::R as isize,
    East = Orientation::V as isize | Side::L as isize,
    West = Orientation::V as isize | Side::R as isize,
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

    fn adopt(&mut self, child: Rc<RefCell<Region>>, s: Side) -> Option<Rc<RefCell<Region>>> {
        if let RegionKind::Split{subregion,..} = &mut self.kind {
            subregion[s as usize].replace(child)
        } else {
            panic!("only splits can adopt nodes")
        }
    }

    #[inline]
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

    #[inline]
    fn clone_from_container(&self) -> Option<Rc<RefCell<Region>>> {
        Some(Rc::clone(&self.container
                       .as_ref()?
                       .upgrade()
                       .unwrap()
                       .borrow()
                       .subregion()[self.from().unwrap() as usize]
                       .as_ref()
                       .unwrap()))
    }

    fn replace(&self, new: Rc<RefCell<Region>>, s: Side) -> Option<Rc<RefCell<Region>>> {
        self.container
            .as_ref()?
            .upgrade()
            .unwrap()
            .borrow_mut()
            .adopt(new, s)
    }

    pub fn split(&mut self, o: Orientation) -> Rc<RefCell<Region>> {
        let mut cont: Option<Weak<RefCell<Region>>> = None;
        let new = Rc::new_cyclic(|this| { 
            cont = Some(this.clone());
            RefCell::new(Region {
                kind: RegionKind::Split{
                    subregion: [self.clone_from_container(),None],
                    fact: 0.5,
                    o,
                },
                container: self.container.clone(),
                tags: self.tags,
            })
        }
        );
        self.replace(new.clone(), self.from().unwrap_or(Side::L));
        self.container = cont;
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
