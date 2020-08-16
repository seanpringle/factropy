use <lib.scad>

module base() {
	difference() {
		rbox([8,8,3], 0.05, $fn=8);
		translate([0,0,2.51]) sphere(r=4);
	}
}

module ring1() {
	rotate([0,90,0]) difference() {
		cyl(3.9,3.9,0.5);
		cyl(3.7,3.7,0.6);
	}
}

module ring2() {
	rotate([0,0,90]) rotate([0,90,0]) difference() {
		cyl(3.6,3.6,0.5);
		cyl(3.4,3.4,0.6);
	}
}

module ring3() {
	rotate([0,90,0]) rotate([0,90,0]) difference() {
		cyl(3.3,3.3,0.5);
		cyl(3.1,3.1,0.6);
	}
}

hd = 72;
ld = 8;

base($fn=hd);
//ring1($fn=hd);
//ring2($fn=hd);
//ring3($fn=hd);