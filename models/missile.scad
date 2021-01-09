use <lib.scad>

module chassis() {
	rotate([90,0,0]) union() {
		cyl(0.25, 0.25, 2.0);
		translate([0,0,0.5]) box([1.0, 0.1, 0.5]);
		translate([0,0,-0.5]) box([1.0, 0.1, 0.5]);
		translate([0,0,-0.5]) box([0.1, 1.0, 0.5]);
	};
}

chassis($fn=72);
