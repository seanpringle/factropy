use <lib.scad>

module small() {
	union() {
		translate([0,0,3.0]) cyl(0.1, 0.1, 6.0);
		translate([0,0,6.0]) rotate([0,90,0]) cyl(0.05, 0.05, 1.0);
	}
}

small($fn=36);