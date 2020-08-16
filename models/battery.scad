use <lib.scad>

module terminal() {
	translate([0,0,0.425]) cyl(0.04,0.04,0.1);
}

module cap() {
	translate([0,0,0.4]) box([0.31,0.51,0.075], 0.01);
}

module body() {
	translate([0,0,0.2]) box([0.3,0.5,0.4], 0.01);
}

d = 72;

//fillet(0.01, $fn=d/8) body($fn=d);
//body($fn=d);
//fillet(0.01, $fn=d/8) cap($fn=d);
//cap($fn=d);
terminal($fn=8);