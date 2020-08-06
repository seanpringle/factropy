use <lib.scad>;

// Module randomRock : draws a random rock
module randomRock(radius,corner_size,number_of_corners){
  // Draw rock
  hull(){
    // Make a hull..
    for(i=[0:number_of_corners]){
      // random position in 'sphere'
      y = rands(0,360,1)[0];
      z = rands(0,360,1)[0];
      r = rands(corner_size*0.5,corner_size,1)[0];

      rotate([0,y,z])

      // translate radius (minus half of corner size)
      translate([radius-corner_size*0.5,0,0])

      // draw cube as corner
      rotate([rands(0,360,1)[0],rands(0,360,1)[0],rands(0,360,1)[0]]) // random rotation
      //cube(rands(corner_size*0.5,corner_size,1)[0],center = true);
      cylinder(r1=r, r2=0, h=r, $fn=3);
    }
  }
}

// : More corners --> More 'spheric' central stone
number_of_corners = rands(15,20,1)[0]; // [5:30]

// : Deforms central stone in vertical
stone_scale = 0.6; // [0.2:0.1:3]

// Number of medium size stones
medium_stones = 5; //rands(5,10,1)[0]; // [5:30]

// Number of small stones
small_stones = 0; //rands(8,16,1)[0]; // [5:30]

/* [Hidden] */
radius = 20;
corner_size = 2;

// Example
translate([0,0,0.5]) scale([0.05,0.05,0.05])
	translate([0,0,-10]) intersection(){
	  union(){

	    // big central stone
	    scale([0.6,0.6,stone_scale])

	    randomRock(radius,corner_size,number_of_corners);

	    // medium size stones
	    if(medium_stones>0){
	      for(i=[1:medium_stones]){
	        // random position in 'circle'
	        z = rands(0,360,1)[0];
	        rotate([0,0,z])

	        // translate radius
	        translate([rands(radius*0.4,radius*0.5,1)[0],0,0])
	        	randomRock(rands(radius*0.3,radius*0.5,1)[0],corner_size*0.5,10);
	      }
	    }

	    // small stones
	    if(small_stones>0){
	      for(i=[0:small_stones]){
	        // random position in 'circle'
	        z = rands(0,360,1)[0];
	        rotate([0,0,z])

	        // translate radius
	        translate([rands(radius*0.5,radius*0.9,1)[0],0,0])
	        	randomRock(rands(radius*0.1,radius*0.3,1)[0],corner_size*0.5,10);
	      }
	    }

	  }
	  // cut half of figure
	  //translate([-50,-50,0]) cube(100);
	}