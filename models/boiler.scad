use <lib.scad>

module chassis() {
	box([3,2,2]);
}

hd=72;
ld=8;

chassis($fn=hd);
//chassis($fn=ld);
