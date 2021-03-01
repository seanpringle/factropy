use <lib.scad>

module straight() {
	rotate([180/$fn,0,0]) rotate([0,90,0]) cyl(0.4, 0.4, 1.0);
}

module cross() {
	union() {
		rotate([180/$fn,0,0]) rotate([0,90,0]) cyl(0.4, 0.4, 1.0);
		rotate([0,180/$fn,0]) rotate([90,0,0]) cyl(0.4, 0.4, 1.0);
	}
}

module elbow() {
	union() {
		translate([0.25,0,0]) rotate([180/$fn,0,0]) rotate([0,90,0]) cyl(0.4, 0.4, 0.5);
		translate([0,0.25,0]) rotate([0,180/$fn,0]) rotate([90,0,0]) cyl(0.4, 0.4, 0.5);
		sphere(r=0.4);
	}
}

module tee() {
	union() {
		translate([0.25,0,0]) rotate([180/$fn,0,0]) rotate([0,90,0]) cyl(0.4, 0.4, 0.5);
		translate([0,0.25,0]) rotate([0,180/$fn,0]) rotate([90,0,0]) cyl(0.4, 0.4, 0.5);
		translate([0,-0.25,0]) rotate([0,180/$fn,0]) rotate([90,0,0]) cyl(0.4, 0.4, 0.5);
	}
}

module flange() {
	difference() {
		cyl(0.475, 0.475, 0.1);
		for (i = [0:45:360])
			#rotate([0,0,i]) translate([0.425,0,0]) cyl(0.025, 0.025, 0.11);
	}
}

module ground() {
	union() {
		translate([-0.45,0,-0.45]) rotate([90,0,0]) intersection() {
			rotate_extrude(convexity=10) translate([0.45,0,0]) circle(r=0.4);
			translate([0.5,0.5,0]) box([1,1,1]);
		}
		translate([-0.45,0,0]) rotate([0,90,0]) flange();
		translate([0,0,-0.45]) flange();
	}
}

module item() {
	rotate([0,90,0]) difference() {
		union() {
			rotate([180/$fn,0,0]) rotate([0,90,0]) cyl(0.375, 0.375, 0.9);
			translate([0.4,0,0]) rotate([0,90,0]) flange();
			translate([-0.4,0,0]) rotate([0,90,0]) flange();
		}
		rotate([180/$fn,0,0]) rotate([0,90,0]) cyl(0.3, 0.3, 1.1);
	}
}

hd=72;
ld=8;

//straight($fn=hd);
//straight($fn=ld);
//cross($fn=hd);
//cross($fn=ld);
//elbow($fn=hd);
//elbow($fn=ld);
//tee($fn=hd);
//tee($fn=ld);
//ground($fn=hd);
//ground($fn=ld);
item($fn=hd);
//item($fn=ld);
