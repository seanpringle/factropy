use <lib.scad>

bounds = [2,3,2];
body = [1.8, 2.75, 1.2];

module chassisEngineer() {
	difference() {
		union() {
			rbox(body, 0.05, $fn=8);
			translate([0,0,-0.4]) rbox([x(body)*0.5, y(body)*0.8, z(body)], 0.05, $fn=8);
			translate([0,0,0.15]) cyl(0.5, 0.5, 1.0, $fn=24);
			translate([0,-1,0.45]) rbox([1.5,1,0.5], 0.05, $fn=8);
		};
		#translate([0,0,0.3]) cyl(0.45, 0.45, 1.0, $fn=24);
	};
}

module chassisHauler() {
	difference() {
		union() {
			rbox(body, 0.05, $fn=8);
			translate([0,0,-0.4]) rbox([x(body)*0.5, y(body)*0.8, z(body)], 0.05, $fn=8);
		};
	};
}

module wheel() {
	rotate([0,90,0]) cyl(0.25, 0.25, 0.35, $fn=24);
}

chassisEngineer();
//wheel();