use <lib.scad>

bounds = [5.99, 5.99, 3];

module oring() {
	difference() {
		cyl(0.51, 0.51, 0.03);
		cyl(0.48, 0.48, 0.97);
	}
}

module piston() {
	difference() {
		cyl(0.5, 0.5, 1.0);
		translate([0,0,0.1]) oring();
		translate([0,0,0.2]) oring();
		translate([0,0,0.3]) oring();
		translate([0,0,0.4]) oring();
	}
}

module frame() {
	difference() {
		difference() {
			box(bounds);
			rotate([0,0,000]) translate([bounds.x/1.57,0,bounds.z/2]) rotate([0,70,0]) box(bounds);
			rotate([0,0,090]) translate([bounds.x/1.57,0,bounds.z/2]) rotate([0,70,0]) box(bounds);
			rotate([0,0,180]) translate([bounds.x/1.57,0,bounds.z/2]) rotate([0,70,0]) box(bounds);
			rotate([0,0,270]) translate([bounds.x/1.57,0,bounds.z/2]) rotate([0,70,0]) box(bounds);
		}
		for (i = [0:3])
			rotate([0,0,90*i]) translate([2.5,0,0.5]) rotate([0,70,0]) cyl(0.6, 0.6, 0.5);

		for (i = [0:3]) for (j = [0:3])
			rotate([0,0,90*i]) translate([3.0-j/8,1.5,j/4+0.05]) rotate([0,20,0]) box([1,1,0.1]);

		for (i = [0:3]) for (j = [0:3])
			rotate([0,0,90*i]) translate([3.0-j/8,-1.5,j/4+0.05]) rotate([0,20,0]) box([1,1,0.1]);

		translate([0,0,1.5]) box([4,4,1]);

		translate([1,0,1]) rotate([90,0,0]) cyl(0.75, 0.75, 3.0);
		translate([-1,0,1]) cyl(0.55, 0.55, 1.0);
		translate([-1,1.2,1]) cyl(0.55, 0.55, 1.0);
		translate([-1,-1.2,1]) cyl(0.55, 0.55, 1.0);
	}
}

hd = 72;
ld = 8;

fillet(0.01, $fn=hd/8) frame($fn=hd);
//frame($fn=hd);
//frame($fn=ld);

//translate([-1,0,1]) piston($fn=hd);
//translate([-1,0,1]) piston($fn=ld);
