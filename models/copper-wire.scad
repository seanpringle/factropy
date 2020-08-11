use <lib.scad>

module roll() {
	cyl(0.25, 0.25, 0.5);
}

module end() {
	cyl(0.3, 0.3, 0.01);
}

d = 8;

//roll($fn=d);
end($fn=d);
