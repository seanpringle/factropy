use <lib.scad>

bounds = [1,1,1];
body = [0.99, 0.99, 0.99];

module base() {
	difference() {
		union() {
			box([x(body)/2, y(body)/2, 0.5]);
			translate([0,0,z(body)/4]) box([x(body), y(body), 0.1]);
		}
		#translate([0,0,z(body)/4+0.05]) box([x(body)-0.1, y(body)+0.1, 0.1]);
	}
}

module roller() {
	translate([0,0,z(body)/4])
		rotate([0,90,0]) cyl(0.05, 0.05, x(body)-0.11, $fn=24);
}

module chevron() {
	//translate([0,0,z(body)/4-0.02])
		rotate([0,0,-90]) cyl(0.35, 0.35, 0.05, $fn=3);
}

//base();
//roller();
chevron();