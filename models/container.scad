use <lib.scad>

bounds = [2,5,2];
body = [1.95, 4.95, 2];

module sideCutOuts() {
	union() {
		translate([-0.05,0,0]) rotate([0,0,30]) cyl(0.1, 0.1, z(body)-0.2, $fn=6);
		for (i = [1:11])
			translate([0,0.2*i,0]) translate([-0.05,0,0]) rotate([0,0,30]) cyl(0.1, 0.1, z(body)-0.2, $fn=6);
		for (i = [1:11])
			translate([0,-0.2*i,0]) translate([-0.05,0,0]) rotate([0,0,30]) cyl(0.1, 0.1, z(body)-0.2, $fn=6);
	}
}

module hd() {
	difference() {
		rbox(body,0.01,$fn=12);
		rotate([0,000,0]) translate([-x(body)/2,0,0]) sideCutOuts();
		rotate([0,090,0]) translate([-x(body)/2,0,0]) sideCutOuts();
		rotate([0,180,0]) translate([-x(body)/2,0,0]) sideCutOuts();
		//rotate([0,270,0]) translate([-x(body)/2,0,0]) sideCutOuts();
	}
}

module ld() {
	box(body);
}

ld();
//hd();