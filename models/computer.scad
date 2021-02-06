use <lib.scad>

module rack() {
	translate([0,0,1.0]) difference() {
		box([1.0, 1.0, 2.0]);
		translate([0,-0.9,0]) box([0.9, 0.9, 1.8]);
	}
}

rack($fn=36);