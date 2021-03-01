use <lib.scad>

module square() {
	difference() {
		box([1000,1000,1]);
		box([960,960,20]);
	}
}

square();
