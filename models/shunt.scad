use <lib.scad>

module frame() {
	union() {
		translate([0,0,0.1]) box([0.99, 0.99, 0.2]);
		translate([0,0,1.95]) difference() {
			box([0.99, 0.99, 0.1]);
			box([0.9, 0.9, 0.15]);
		}
		translate([0.475,0.475,1.0]) box([0.04, 0.04, 2.0]);
		translate([0.475,-0.475,1.0]) box([0.04, 0.04, 2.0]);
		translate([-0.475,0.475,1.0]) box([0.04, 0.04, 2.0]);
		translate([-0.475,-0.475,1.0]) box([0.04, 0.04, 2.0]);
	}
}

module platform() {
	translate([0,0,1.95]) difference() {
		box([0.85, 0.85, 0.1]);
		translate([0,0,0.05]) box([0.8, 0.8, 0.1]);
	}
}

module surface() {
	translate([0,0,1.995]) box([0.75, 0.75, 0.01]);
}

module telescope1() {
	#translate([0,0,0.5]) cyl(0.15, 0.15, 0.65);
}

module telescope2() {
	#translate([0,0,1.05]) cyl(0.125, 0.125, 0.65);
}

module telescope3() {
	#translate([0,0,1.6]) cyl(0.10, 0.10, 0.65);
}

d = 8;

//fillet(0.01, $fn=$fn/8) frame($fn=d);
frame($fn=d);

//fillet(0.01, $fn=$fn/8) platform($fn=d);
platform($fn=d);
surface($fn=d);

telescope1($fn=d);
telescope2($fn=d);
telescope3($fn=d);
