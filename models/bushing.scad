use <lib.scad>

module bushing() {
	translate([0,0,0.25]) difference() {
		cyl(0.25,0.25,0.5);
		cyl(0.2,0.2,0.6);
	}
}

d = 72;

fillet(0.01, $fn=d/8) bushing($fn=d);
//bushing($fn=d);