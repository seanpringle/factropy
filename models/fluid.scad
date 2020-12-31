
module droplet() {
	union() {
	  sphere(r = 0.3);
	  translate([0, 0, 0.3 * sin(30)])
	    cylinder(h = 0.6, r1 = 0.3 * cos(30), r2 = 0);
	}
}

droplet($fn=72);