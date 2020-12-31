use <lib.scad>

module chassis() {
	translate([0,0,1.5]) box([3,3,3]);
}

module pipe() {
	translate([0,0,-2.5]) cyl(0.5, 0.5, 5);
}

hd=72;
ld=8;

//chassis($fn=hd);
//chassis($fn=ld);
//pipe($fn=hd);
pipe($fn=ld);