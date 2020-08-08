use <lib.scad>

bounds = [3.95,3.95,4];

module tunnel() {
	union() {
		rotate([90,0,0]) cyl(0.6,0.6,1);
		translate([0,0,-0.4]) box([1.2, 1, 0.8]);
	}
}

module fire() {
	translate([0,-1.6,0]) intersection() {
		tunnel();
		box([2,0.1,2]);
	}
}

module smoke() {
	translate([0,0,1.8]) box([1,1,0.1]);
}

module pyramid() {
	difference() {
		box(bounds);
		rotate([0,0,000]) translate([bounds.x/1.4,0,bounds.z/2]) rotate([0,70,0]) box([x(bounds)*2,y(bounds),z(bounds)]);
		rotate([0,0,090]) translate([bounds.x/1.4,0,bounds.z/2]) rotate([0,70,0]) box([x(bounds)*2,y(bounds),z(bounds)]);
		rotate([0,0,180]) translate([bounds.x/1.4,0,bounds.z/2]) rotate([0,70,0]) box([x(bounds)*2,y(bounds),z(bounds)]);
		rotate([0,0,270]) translate([bounds.x/1.4,0,bounds.z/2]) rotate([0,70,0]) box([x(bounds)*2,y(bounds),z(bounds)]);
	}
}

module furnace() {
	difference() {
		union() {
			difference() {
				pyramid();
				intersection() {
					difference() {
						scale(1.01) pyramid();
						scale(0.98) pyramid();
					}
					for (i = [0:5]) {
						translate([0,0,i/2-1]) box([5,5,0.05]);
					}
				}
			}
			translate([0,-1.3,0]) union() {
				rotate([90,0,0]) cyl(0.8,0.8,1);
				translate([0,0,-0.8]) box([1.6, 1, 1.6]);
			}
		}
		translate([0,-2.1,0]) tunnel();
		translate([0,0,2.3]) box([1,1,1]);
	}
}

ld = 8;
hd = 72;

fillet(0.01, $fn=hd/8) furnace($fn=hd);
//furnace($fn=hd);
//furnace($fn=ld);
//fire($fn=hd);
//fire($fn=ld);
//smoke($fn=hd);
//smoke($fn=ld);
