use <lib.scad>

module base() {
	difference() {
		translate([0,0,1.0-0.05]) box([1.0, 1.0, 0.1]);
		translate([0,0,1.0]) box([0.9, 1.1, 0.1]);
	}
}

module loaderBase() {
	difference() {
		translate([0,0,1.25]) box([1.0, 1.0, 0.7]);
		translate([0,0,1.25]) box([0.9, 1.1, 0.6]);
	}
}

module pillar() {
	translate([0,0,0.45]) box([0.5, 0.5, 0.9]);
}

module belt() {
	translate([0,0,0.995]) box([0.85, 1.0, 0.01]);
}

module ridge() {
	translate([0,0.5,1.005]) box([0.8, 0.01, 0.01]);
}

//fillet(0.01, $fn=12) base();
//base();
//fillet(0.01, $fn=12) loaderBase();
loaderBase();
//fillet(0.01, $fn=12) pillar();
//pillar();
//belt();
//ridge();