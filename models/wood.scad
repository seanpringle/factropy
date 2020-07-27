use <lib.scad>

r = 0.2;

translate([0,0,0.39]) rotate([-90,0,0]) union() {
	translate([0.197,0.197,0]) cyl(r,r,0.9,$fn=24);
	translate([-0.197,0.197,0]) cyl(r,r,0.9,$fn=24);
	translate([0,-0.14,0]) cyl(r,r,0.9,$fn=24);
}