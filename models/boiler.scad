use <lib.scad>

module rivet() {
	union() {
		translate([0,0,0.005]) sphere(0.019);
		translate([0,0,-0.015]) cyl(0.025, 0.025, 0.05);
	}
}

module flange() {
	rotate([0,90,0]) difference() {
		cyl(0.85,0.85,0.1);
		difference() {
			cyl(0.9,0.9,0.01);
			cyl(0.82,0.82,0.05);
		}
	}
}

module firebox() {
	union() {
		hull() {
			translate([1.1,0,0]) rotate([0,90,0]) rotate_extrude(convexity=10) translate([0.6,0,0]) circle(r=0.2);
			translate([-1.1,0,0]) rotate([0,90,0]) rotate_extrude(convexity=10) translate([0.6,0,0]) circle(r=0.2);
		}
		translate([1.1,0,0]) flange();
		translate([-1.1,0,0]) flange();
		//for (i=[0:1:11]) translate([1.3,0,0]) rotate([i*30,0,0]) translate([0,0,0.55]) rotate([0,90,0]) rivet();
		//for (i=[0:1:11]) translate([-1.3,0,0]) rotate([i*30,0,0]) translate([0,0,0.55]) rotate([0,-90,0]) rivet();
		//for (i=[0:1:17]) translate([0.9,0,0]) rotate([i*20,0,0]) translate([0,0,0.8]) rivet();
		//for (i=[0:1:17]) translate([-0.9,0,0]) rotate([i*20,0,0]) translate([0,0,0.8]) rivet();
		//for (i=[0:1:17]) translate([0.3,0,0]) rotate([i*20,0,0]) translate([0,0,0.8]) rivet();
		//for (i=[0:1:17]) translate([-0.3,0,0]) rotate([i*20,0,0]) translate([0,0,0.8]) rivet();
	}
}

module stack() {
	translate([0.6,0,0.5]) intersection() {
		translate([0,0,-0.5]) union() {
			difference() {
				cyl(0.3,0.3,2.0);
				cyl(0.25,0.25,2.1);
			}
			translate([0,0,0.1]) intersection() {
				rotate([0,90,0]) cyl(0.75,0.75,1.0);
				cyl(0.44,0.44,10);
			}
		}
		box([1,1,1]);
	}
}

module cover() {
	translate([0.6,0,0.9]) cyl(0.25,0.25,0.1);
}

module chassis() {
	translate([0,0,-0.5]) box([2.9,1.9,1.0]);
}

hd=72;
ld=8;

FN=ld;

//chassis($fn=FN);

//difference() {
//	firebox($fn=FN);
//	chassis($fn=FN);
//}

//stack($fn=FN);
cover($fn=FN);
//rivet($fn=FN/4);