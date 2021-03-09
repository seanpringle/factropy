use <lib.scad>

module plate() {
	translate([0,0,0.025]) box([0.6,0.6,0.05]);
}

//fillet(0.01, $fn=16) plate();
plate();
