use <lib.scad>

bounds = [1.99, 1.99, 3];

module base() {
	translate([0,0,0.125]) cyl(0.75, 0.75, 0.25, $fn=24);
}

module pillar() {
	fillet(0.05) translate([0,0.1,0]) difference() {
		translate([0,-0.25,1.425]) box([0.5, 1.3, 2.5]);
		translate([0,-0.9,0.5]) rotate([30,0,0]) box([0.6, 1.0, 2.75]);
		translate([0,0.5,0]) scale([1.1,1,1]) difference() {
			translate([0,-0.4,1.625]) box([0.5, 1.2, 3]);
			translate([0,-0.95,0.5]) rotate([30,0,0]) box([0.5, 1.0, 2.75]);
		}
	}
}

module telescope1() {
	translate([0,-0.5,2.4]) rotate([90,0,0]) cyl(0.2, 0.2, 0.75, $fn=24);
}

module telescope2() {
	translate([0, 0.1,2.4]) rotate([90,0,0]) cyl(0.18, 0.18, 0.75, $fn=24);
}

module telescope3() {
	translate([0, 0.7,2.4]) rotate([90,0,0]) cyl(0.16, 0.16, 0.75, $fn=24);
}

module grip() {
	union() {
		translate([0,	1.2,2.4]) sphere(0.25, $fn=24);
		translate([0,	1.2,2.15]) cyl(0.25, 0.25, 0.1, $fn=24);
	}
}

//base();
pillar();
//telescope1();
//telescope2();
//telescope3();
//grip();