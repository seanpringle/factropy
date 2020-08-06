use <lib.scad>

module power() {
	union() {
		rotate([0,90,0]) cyl(0.005, 0.005, 1.0);
	}
}

power($fn=36);