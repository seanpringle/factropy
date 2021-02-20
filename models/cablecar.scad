use <lib.scad>

module mast() {
	union() {
		translate([0,0,5.0]) cyl(0.25, 0.25, 10.0);
		translate([0,0,10.0]) box([4.0, 0.1, 0.1]);
	}
}

module bucket() {
	translate([0,0,1]) union() {
		cyl(1,1,2);
		translate([0,0,1.5]) cyl(0.1,0.1,3.0);
	}
}

module terminusBase() {
	translate([0,0,1]) box([5,5,2]);
}

module towerBase() {
	translate([0,0,1]) box([3,3,2]);
}

//mast($fn=72);
mast($fn=12);
