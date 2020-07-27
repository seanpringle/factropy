use <lib.scad>

s=0.2;

module ingot() {
	fillet(0.02, $fn=16) scale([s,s,s]) scale([3,1,1]) rotate([0,0,45]) cyl(1, 0.8, 1, $fn=4);
}

translate([0,-0.175,0.1]) ingot();
translate([0,0,0.3]) ingot();
translate([0,0.175,0.1]) ingot();
