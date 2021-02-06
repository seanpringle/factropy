use <lib.scad>

module base() {
	translate([0,0,0.125]) union() {
		box([0.25, 0.25, 0.25]);
		translate([0,0,0.25]) cyl(0.025, 0.025, 0.25);
		translate([0,0,0.35]) sphere(0.05);
	}
}

base($fn=8);
