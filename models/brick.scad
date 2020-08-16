use <lib.scad>

module body() {
	translate([0,0,0.15]) difference() {
		if ($fn > 8) rbox([0.6, 0.4, 0.3], 0.01, $fn=round($fn/8)); else box([0.6, 0.4, 0.3]);
		#translate([0.18,0.1,0]) cyl(0.05, 0.05, 1.0);
		#translate([0.0,0.1,0]) cyl(0.05, 0.05, 1.0);
		#translate([-0.18,0.1,0]) cyl(0.05, 0.05, 1.0);
		#translate([0.18,-0.1,0]) cyl(0.05, 0.05, 1.0);
		#translate([0.0,-0.1,0]) cyl(0.05, 0.05, 1.0);
		#translate([-0.18,-0.1,0]) cyl(0.05, 0.05, 1.0);
	}
}

d = 8;
body($fn=d);
