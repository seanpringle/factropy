use <lib.scad>

module tank() {
	translate([0,0,-2]) intersection() {
		sphere(r=5);
		cyl(2.5,2.5,10);
		translate([0,0,3.5]) box([5,5,3]);
	}
}

hd=72;
ld=12;

//tank($fn=hd);
tank($fn=ld);