use <lib.scad>

bounds = [4.99,9.99,5];

module chassis() {
	difference() {
		rbox(bounds, 0.01, $fn=12);
		translate([0,-y(bounds)/1.3,z(bounds)/1]) rotate([-30,0,0]) box([x(bounds)*1.1, y(bounds), z(bounds)*1.5]);
		translate([0,-y(bounds)/4.2,0]) difference() {
			#rotate([90,0,0]) cyl(x(bounds)/2.5, x(bounds)/2.5, y(bounds)/2, $fn=24);
			#translate([0,-y(bounds)/3.5,-z(bounds)/4]) box([x(bounds), x(bounds)/2.5, y(bounds)/3]);
		}
	}
}

chassis();