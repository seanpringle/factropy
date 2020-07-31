use <lib.scad>

module blade() {
	translate([0.25,0,0]) rotate([30,0,0]) scale([2,1,1]) cyl(0.125, 0.125, 0.05);
}

module hd() {
	union() {
		cyl(0.125, 0.125, 0.25, $fn=24);
		rotate([0,0,000]) blade($fn=24);
		rotate([0,0,090]) blade($fn=24);
		rotate([0,0,180]) blade($fn=24);
		rotate([0,0,270]) blade($fn=24);
	}
}

module ld() {
	union() {
		cyl(0.125, 0.125, 0.25, $fn=8);
		rotate([0,0,000]) blade($fn=8);
		rotate([0,0,090]) blade($fn=8);
		rotate([0,0,180]) blade($fn=8);
		rotate([0,0,270]) blade($fn=8);
	}
}

hd();
//ld();