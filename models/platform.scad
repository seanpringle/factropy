use <lib.scad>

bounds = [10,10,1];
body = [9.99, 9.99, 0.99];

difference() {
	rbox(body,0.01,$fn=12);
	for (x = [-4:4])
		#translate([x,0,body.z/2]) rotate([90,0,0]) cyl(0.01,0.01,bounds.y,$fn=12);
	for (y = [-4:4])
		#translate([0,y,body.z/2]) rotate([0,90,0]) cyl(0.01,0.01,bounds.y,$fn=12);
}
