use <lib.scad>

bounds = [1,1,1];
body = [0.9, 0.9, 0.9];

module chassis() {
	sphere(r=0.9/2, $fn=36);
}

chassis();
//wheel();