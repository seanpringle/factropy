use <lib.scad>

module base() {
	translate([0,0,0.1]) cyl(0.45, 0.45, 0.2);
}

module pillar() {
	translate([0,-0.30,0.9]) rbox([0.32,0.2,1.8],0.01,$fn=$fn/4);
}

module telescope1() {
	#translate([0,0,1.625]) rotate([90,0,0]) cyl(0.15, 0.15, 0.5);
}

module telescope2() {
	#translate([0,0,1.625]) rotate([90,0,0]) cyl(0.125, 0.125, 0.5);
}

module telescope3() {
	#translate([0,0,1.625]) rotate([90,0,0]) cyl(0.10, 0.10, 0.5);
}

module telescope4() {
	#translate([0,0,1.625]) rotate([90,0,0]) cyl(0.075, 0.075, 0.5);
}

module telescope5() {
	#translate([0,0,1.625]) rotate([90,0,0]) cyl(0.05, 0.05, 0.5);
}

module grip() {
	union() {
		translate([0,0,1.67]) sphere(0.2);
		translate([0,0,1.52]) cyl(0.2, 0.2, 0.1);
	}
}

d = 8;

//base($fn=d);
//pillar($fn=d);
//telescope1($fn=d);
//telescope2($fn=d);
//telescope3($fn=d);
//telescope4($fn=d);
//telescope5($fn=d);
grip($fn=d);