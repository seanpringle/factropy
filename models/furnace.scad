use <lib.scad>

bounds = [3.95,3.95,4];

module furnace() {
	difference() {
		union() {
			difference() {
				box(bounds);
				rotate([0,0,000]) translate([bounds.x/1.4,0,bounds.z/2]) rotate([0,70,0]) box([x(bounds)*2,y(bounds),z(bounds)]);
				rotate([0,0,090]) translate([bounds.x/1.4,0,bounds.z/2]) rotate([0,70,0]) box([x(bounds)*2,y(bounds),z(bounds)]);
				rotate([0,0,180]) translate([bounds.x/1.4,0,bounds.z/2]) rotate([0,70,0]) box([x(bounds)*2,y(bounds),z(bounds)]);
				rotate([0,0,270]) translate([bounds.x/1.4,0,bounds.z/2]) rotate([0,70,0]) box([x(bounds)*2,y(bounds),z(bounds)]);
			}
			translate([0,-1.3,0]) union() {
				rotate([90,0,0]) cyl(0.8,0.8,1);
				translate([0,0,-0.8]) box([1.6, 1, 1.6]);
			}
		}
		difference() {
			translate([0,-1,0]) union() {
				rotate([90,0,0]) cyl(0.6,0.6,3);
				translate([0,0,-0.4]) box([1.2, 3, 0.8]);
			}
		}
		translate([0,0,1.5]) box([1,1,4]);
	}
}

furnace($fn=8);