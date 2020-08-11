use <lib.scad>

bounds = [1,1,1];
body = [0.9, 0.9, 0.9];

module blade() {
	translate([0.1,0,0.03]) rotate([30,0,0]) scale([3,1,1]) cyl(0.03, 0.03, 0.01);
}

module rotor() {
	translate([0,0,0.01]) union() {
		cyl(0.03, 0.03, 0.1);
		blade();
		rotate([0,0,180]) blade();
	}
}

module spars() {
	union() {
		rotate([0,90,0]) cyl(0.02, 0.02, 0.8);
		rotate([90,0,0]) cyl(0.02, 0.02, 0.8);
	}
}

module chassis() {
	union() {
		difference() {
			sphere(r=0.15);
			translate([0,0,-0.35]) box([0.5,0.5,0.5]);
			translate([0,0,0.35]) box([0.5,0.5,0.5]);
		}
		rotate([0,90,0]) translate([0,0,0.05]) cyl(0.05, 0.05, 0.25);
	}
}



d = 8;

rotate([0,0,-90]) chassis($fn=d);
//spars($fn=d);
//rotor($fn=d);
