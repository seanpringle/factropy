use <lib.scad>

module chassis() {
	rotate([90,0,0]) union() {
		cyl(0.02, 0.01, 0.1);
	};
}

chassis($fn=12);
