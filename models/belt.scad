use <lib.scad>

module base() {
	difference() {
		translate([0,0,1000-50]) box([1000, 1000, 100]);
		translate([0,0,1000]) box([900, 1100, 100]);
	}
}

module loaderBase() {
	difference() {
		translate([0,0,1250]) box([1000, 1000, 700]);
		translate([0,0,1250]) box([900, 1100, 600]);
	}
}

module pillar() {
	translate([0,0,450]) box([500, 500, 900]);
}

module belt() {
	translate([0,0,995]) box([850, 1000, 10]);
}

module ridge() {
	translate([0,0,1005]) box([800, 10, 10]);
}

module baseRight() {
	translate([0,0,1000-50]) union() {
		translate([500,-500,0]) intersection() {
			cyl(50, 50, 100);
			translate([-25,25,0]) box([50, 50, 100]);
		}
		intersection() {
			translate([500,-500,0]) difference() {
				cyl(1000, 1000, 100);
				translate([0,0,50]) cyl(950, 950, 100);
			}
			box([1000, 1000, 200]);
		}
	}
}

module baseLeft() {
	mirror([1,0,0]) baseRight();
}

module beltRight() {
	translate([0,0,995]) intersection() {
		translate([500,-500,0]) difference() {
			cyl(925, 925, 10);
			cyl( 75,  75, 20);
		}
		box([1000, 1000, 200]);
	}
}

module beltLeft() {
	mirror([1,0,0]) beltRight();
}

module baseSlope() {
	translate([0,0,950]) intersection() {
		rotate([45,0,0]) difference() {
			box([1000, 3000, 100]);
			translate([0,0,50]) box([900, 3100, 100]);
		};
		box([1000,1000,2000]);
	};
}

module beltSlope() {
	translate([0,0,995]) intersection() {
		rotate([45,0,0]) box([850, 2000, 10]);
		box([1000,1000,2000]);
	}
}

//fillet(10, $fn=12) base();
//base();
//fillet(10, $fn=12) loaderBase();
//loaderBase();
//fillet(10, $fn=12) pillar();
//pillar();
//belt();
//ridge();

//baseRight($fn=72);
//baseRight($fn=36);

//beltRight($fn=72);
//beltRight($fn=36);

//baseLeft($fn=72);
//baseLeft($fn=36);

//beltLeft($fn=72);
//beltLeft($fn=36);

//baseSlope();
beltSlope();