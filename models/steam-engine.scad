use <lib.scad>;

module axel() {
	rotate([0,90,0]) difference() {
		cyl(0.1,0.1,3.75);
	}
}

module spoke() {
	intersection() {
		translate([0.5,0.8,0])
			difference() {
				cyl(1.0,1.0,0.15);
				cyl(0.9,0.9,0.35);
			};

		translate([0,0,0])
			cyl(1.0,1.0,0.25);

		translate([0,1,0])
			cyl(1.0,1.0,0.25);
	}
}

module wheel() {
	union() {
		difference() {
			cyl(1.0,1.0,0.25);
			cyl(0.9,0.9,0.35);
		}
		rotate([0,0,000]) spoke();
		rotate([0,0,045]) spoke();
		rotate([0,0,090]) spoke();
		rotate([0,0,135]) spoke();
		rotate([0,0,180]) spoke();
		rotate([0,0,225]) spoke();
		rotate([0,0,270]) spoke();
		rotate([0,0,315]) spoke();
	}
}

module boiler() {
	union() {
		// boiler
		translate([0,0,2]) rotate([90,0,0]) cyl(1.5,1.5,4.5);
		translate([0,0,2]) rotate([90,0,0]) cyl(0.75,0.75,4.7);

		// piston
		translate([0,-1.5,3.5]) cyl(0.50,0.50,1);
		translate([0, 0.0,3.5]) cyl(0.45,0.45,1);
		translate([0, 1.5,3.5]) cyl(0.40,0.40,1);
	}
}

module saddle() {
	translate([0,0,2.6]) rbox([2.5,4.2,2],0.01,$fn=$fn/4);
}

module foot() {
	translate([0,0,0.75]) rotate([90,30,0]) cyl(1.5,1.5,4,$fn=3);
}

module preview() {
	boiler();
	translate([ 1.75,-1,3]) rotate([0,90,0]) wheel();
	translate([-1.75,-1,3]) rotate([0,90,0]) wheel();
	translate([    0,-1,3]) axel();
}

//preview($fn=72);

//boiler($fn=12);
//saddle($fn=12);
//foot($fn=12);
//wheel($fn=12);
//axel($fn=8);