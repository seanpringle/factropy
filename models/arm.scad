use <lib.scad>

module base() {
	translate([0,0,0.35]) cyl(0.45, 0.45, 0.7);
}

module pillar() {
	translate([0,-0.30,1.5]) rbox([0.32,0.2,2.0],0.01,$fn=$fn/4);
}

module telescope1() {
	#translate([0,-0.2,2.3]) rotate([90,0,0]) cyl(0.15, 0.15, 0.5);
}

module telescope2() {
	#translate([0, 0.19,2.3]) rotate([90,0,0]) cyl(0.125, 0.125, 0.5);
}

module telescope3() {
	#translate([0, 0.6,2.3]) rotate([90,0,0]) cyl(0.10, 0.10, 0.5);
}

module grip() {
	union() {
		translate([0,	1.0,2.3]) sphere(0.2);
		translate([0,	1.0,2.15]) cyl(0.2, 0.2, 0.1);
	}
}

d = 12;

//base($fn=d);
//pillar($fn=d);
//telescope1($fn=d);
//telescope2($fn=d);
//telescope3($fn=d);
grip($fn=d);