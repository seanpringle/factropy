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

hd=72;
ld=8;

//straight($fn=hd);
//straight($fn=ld);
//cross($fn=hd);
//cross($fn=ld);
//elbow($fn=hd);
//elbow($fn=ld);
//tee($fn=hd);
tee($fn=ld);