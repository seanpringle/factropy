use <lib.scad>

union() {
	cyl(0.4, 0.4, 0.25, $fn=24);
	for (i = [1:12])
		rotate([0,0,i*360/12]) translate([0.4,0,0]) scale([1,0.6,1]) cyl(0.1, 0.1, 0.25, $fn=24);
}