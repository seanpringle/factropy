use <lib.scad>

module board() {
	rotate([0,0,180]) difference() {
		box([0.85, 0.85, 0.05]);
		translate([0.35,0.45,0]) box([0.3, 0.3, 0.1]);
		translate([-0.1,0.75,0.04]) box([0.85, 0.85, 0.05]);
		translate([-0.1,0.75,-0.04]) box([0.85, 0.85, 0.05]);
		#for (i = [-20:9])
			translate([i*0.02,0.38,0.0175]) box([0.01, 0.1, 0.01]);
		#for (i = [-20:9])
			translate([i*0.02,0.38,-0.0175]) box([0.01, 0.1, 0.01]);
	}
}

board();
