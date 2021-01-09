use <lib.scad>

module chassis() {
	translate([0,0,0.25]) cyl(0.5, 0.4, 0.5);
}

module dome() {
	translate([0,0,0.5]) sphere(0.4);
}

module barrel() {
	translate([0,0,0.5]) rotate([90,0,0]) translate([0,0,0.25]) cyl(0.05,0.05,0.75);
}

//chassis($fn=72);
dome($fn=72);
//barrel($fn=72);
