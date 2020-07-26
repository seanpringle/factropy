use <lib.scad>

module blade() {
	translate([0.25,0,0]) rotate([30,0,0]) scale([2,1,1]) cyl(0.125, 0.125, 0.05, $fn=24);
}

union() {
	cyl(0.125, 0.125, 0.25, $fn=24);
	rotate([0,0,000]) blade();
	rotate([0,0,090]) blade();
	rotate([0,0,180]) blade();
	rotate([0,0,270]) blade();
}