use <lib.scad>

module base() {
	translate([0,0,0.1]) cyl(0.45, 0.45, 0.2);
}

module platform() {
	translate([0,0,1.95]) box([0.95, 0.95, 0.1]);
}

module telescope1() {
	#translate([0,0,0.5]) cyl(0.15, 0.15, 0.65);
}

module telescope2() {
	#translate([0,0,1.05]) cyl(0.125, 0.125, 0.65);
}

module telescope3() {
	#translate([0,0,1.6]) cyl(0.10, 0.10, 0.65);
}

d = 72;

//fillet(0.01, $fn=$fn/8) base($fn=d);
//base($fn=d);

//fillet(0.01, $fn=$fn/8) platform($fn=d);
//platform($fn=d);

//telescope1($fn=d);
//telescope2($fn=d);
telescope3($fn=d);
