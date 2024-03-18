mod tiles;
use crate::tiles::{Region,RegionKind};
use crate::tiles::Side as S;
use crate::tiles::Orientation as O;
use std::cell::RefCell;
use std::rc::Rc;

fn main() {
    let a = Rc::new(RefCell::new(Region{
        kind: RegionKind::Client{
            window: 2
        },
        container: None,
        tags: 0,
    }));
    let b = Rc::new(RefCell::new(Region{
        kind: RegionKind::Client{
            window: 0
        },
        container: None,
        tags: 0,
    }));
    let r: Rc<RefCell<Region>> = Rc::new(RefCell::new(Region{
        kind: RegionKind::Split{
            subregion: [Some(a.clone()),Some(b.clone())],
            fact: 0.5,
            o: O::V,
        },
        container: None,
        tags: 0,
    }));
    a.borrow_mut().container = Some(Rc::downgrade(&r.clone()));
    b.borrow_mut().container = Some(Rc::downgrade(&r.clone()));
    r.borrow_mut().split(O::H);
    println!("{}", r.borrow());
}
