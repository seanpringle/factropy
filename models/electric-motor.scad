use <lib.scad>

module body() {
	translate([0,0,0.25]) rotate([90,0,0]) cyl(0.2, 0.2, 0.4);
}

module foot() {
	translate([0,0,0.1]) rotate([0,-90,0]) cyl(0.2, 0.2, 0.3, $fn=3);
}

module shaft() {
	translate([0,0,0.25]) rotate([90,0,0]) cyl(0.04, 0.04, 0.5);
}

hd = 72;
ld = 8;

//fillet(0.01, $fn=hd/8) body($fn=hd);
//body($fn=ld);
//fillet(0.01, $fn=hd/8) foot($fn=hd);
//foot($fn=ld);
//fillet(0.01, $fn=hd/8) shaft($fn=hd);
shaft($fn=ld);
