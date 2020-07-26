function x(v) = v[0];
function y(v) = v[1];
function z(v) = v[2];
function add(v1,v2) = [ for (i = [ 0 : len(v1)-1 ]) v1[i] + v2[i] ];
function xv(d) = [d,0,0];
function yv(d) = [0,d,0];
function zv(d) = [0,0,d];

module box(v) { cube(size=v, center=true); }
module cyl(r1,r2,h) { cylinder(r1=r1, r2=r2, h=h, center=true); }

function grow(n,v) = [x(v)+n*2, y(v)+n*2, z(v)+n*2];

module rbox(v, radius=1) {
	cx = x(v)/2-radius;
	cy = y(v)/2-radius;
	cz = z(v)/2-radius;

	hull() {
		translate([cx,cy,cz]) sphere(r=radius);
		translate([cx,cy,-cz]) sphere(r=radius);
		translate([cx,-cy,cz]) sphere(r=radius);
		translate([cx,-cy,-cz]) sphere(r=radius);
		translate([-cx,cy,cz]) sphere(r=radius);
		translate([-cx,cy,-cz]) sphere(r=radius);
		translate([-cx,-cy,cz]) sphere(r=radius);
		translate([-cx,-cy,-cz]) sphere(r=radius);
	}
}

module fillet(r=1) {
  minkowski() {
    difference() {
      children();
      minkowski() {
        difference() {
          minkowski() {
            children();
            sphere(r=r);
          }
          children();
        }
        sphere(r=r);
      }
    }
    sphere(r=r);
  }
}