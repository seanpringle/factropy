use <lib.scad>

bounds = [5.99, 5.99, 3];

difference() {
	box(bounds);
	rotate([0,0,000]) translate([bounds.x/1.57,0,bounds.z/2]) rotate([0,70,0]) box(bounds);
	rotate([0,0,090]) translate([bounds.x/1.57,0,bounds.z/2]) rotate([0,70,0]) box(bounds);
	rotate([0,0,180]) translate([bounds.x/1.57,0,bounds.z/2]) rotate([0,70,0]) box(bounds);
	rotate([0,0,270]) translate([bounds.x/1.57,0,bounds.z/2]) rotate([0,70,0]) box(bounds);
}
