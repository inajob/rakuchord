include <polyScrewThread_r1.scad>;


module screw_hole(r=2.5){
    translate([92/2 - 5, 92/2 + 1,0]) circle(r);
    translate([92/2 - 5, -92/2 + 3,0]) circle(r);

    translate([-92/2 + 5, 92/2 + 1,0]) circle(r);
    translate([-92/2 + 5, -92/2 + 3,0]) circle(r);
}

module base_board(){
      square([92, 100], center = true);
}

module base_board_space(){
    minkowski()
    {
      // base board
      base_board();
      // margin
      circle(1);
    }
}

module hemi_sphere(r){
    difference(){
      sphere(r);
      translate([0,0,-r])cube([r*2,r*2,r*2],center = true);
    }
}

module upper_case_outer(height = 10){
    margin = 5;
    hull(){
        translate([0,0, -height/2]){
            linear_extrude(height = 1){
                minkowski()
                {
                  // base board
                  base_board();
                  // margin
                  circle(margin);
                }
            }
        }
        translate([92/2, 100/2,0]) hemi_sphere(margin);
        translate([92/2, -100/2,0]) hemi_sphere(margin);
        translate([-92/2, 100/2,0]) hemi_sphere(margin);
        translate([-92/2, -100/2,0]) hemi_sphere(margin);
    }
    translate([0,0, 5]) rotate([0,180,0]) linear_extrude(height = height/2 + margin) hull(){
      translate([93/2 ,100/2 + 2,0]) circle(8);
      translate([-93/2 ,100/2 + 2,0]) circle(8);
    }

}

module upper_hole(){
  square([73, 90], center = true);
}

module key1_space(){
  translate([12,100/2 - 5,1]) square([25,5]);
}

module cable_hole(){
  translate([-5,-100/2,1]) square([30,5]);
}

module jack_diff(){
    translate([16,-40,3]) rotate([90,0,0]) linear_extrude(height=20){
       circle(5);
    }
}

*base_board();

// case
*union(){
    difference(){
        upper_case_outer(13);
        translate([0,0,-8])linear_extrude(height=11){
          screw_hole(r=4);
          upper_hole();
          *cable_hole();
          translate([-5,-53,1]) square([30,8]);
        }
    }
}


difference(){
    upper_case_outer();
    translate([0,0,-10])linear_extrude(height=20){
      screw_hole();
      upper_hole();
      cable_hole();
    }
    translate([0,0,-7]) linear_extrude(height=10){
        //base_board_space();
        minkowski(){
            lower_case_outer();
            circle(1);
        }
    }
    translate([0,0,1])linear_extrude(height=3){
      key1_space();
    }
    jack_diff();
}

module sub_board(){
      translate([0,100/2 + 2,-15]) rotate([90,0,0]) square([93, 30], center = true);
}

module lower_case_outer(){
    hull(){
      translate([93/2 ,100/2 + 2,0]) circle(5, $fn=50);
      translate([-93/2 ,100/2 + 2,0]) circle(5, $fn=50);
    }
    square([95, 100], center = true);
    hull(){
        translate([93/2 -1 ,-100/2,0]) circle(2, $fn=50);
        translate([-93/2 +1 ,-100/2,0]) circle(2, $fn=50);
    }
}

module lower_case_inner(){
    square([95 - 6, 100], center = true);
    translate([0,52,0]) square([94, 3], center = true);
    translate([0,50,0]) square([93 - 8, 3], center = true);

    translate([0,55,0]) square([93 - 15, 5], center = true);
    translate([0,-50,0]) square([57, 5], center = true);
}

*sub_board();

module speaker_hold(){
    linear_extrude(height = 20){
        translate([-94/2 + 12,-50 + 5 + 10/2,0]) scale(5) {
            translate([-2,-1,0]) square([1, 2]);
            mirror([0,1,0]) polygon([[-1,-1],[1,-1],[1,0],[0,1],[-1,1]]);
            translate([-2,-2,0]) square([1.5,1]);
        }
    }
}


thread = "none"; //modeled

translate([0,0,-34]){
    difference(){
        union(){
            linear_extrude(height = 33){
                difference(){
                  lower_case_outer();
                  lower_case_inner();
                }
            }
            difference(){
                union(){
                    linear_extrude(height = 2){
                        lower_case_outer();
                    }
                    linear_extrude(height = 5) translate([33,0,0]) circle(7);
                    linear_extrude(height = 5) translate([-33,0,0]) circle(7);
                }
                hg = 8;  // height
                sth = 4; // step height
                clf = 55; // degrees
                cod = 10; // outer diameter
                crs = 1; // resolution

                //translate([33, 0, 20]) hole_threaded(name="M10", thread = thread);
                //translate([-33, 0, 20]) hole_threaded(name="M10", thread = thread);
                translate([33, 0, 0]) screw_thread(cod,sth,clf,8,crs,-2);
                translate([-33, 0, 0]) screw_thread(cod,sth,clf,8,crs,-2);;
                screw_hole();

            }
            linear_extrude(height = 5){
                difference(){
                    square([50+2+4, 53+2+4], center = true);
                    square([50+1, 53+1], center = true);
                }
            }
            speaker_hold();
            mirror([-1,0,0]) speaker_hold();
            linear_extrude(height = 25){
                translate([44,-51,0]){
                    circle(3.5, $fn=20);
                }
            }
            linear_extrude(height = 25){
                translate([49,57,0]){
                    circle(3.5, $fn=20);
                }
            }
        }

        linear_extrude(height = 34){
            screw_hole();
        }
    }
}


module battery_holder(){
    union(){
        linear_extrude(height=3)difference(){
            hull(){
                translate([33,0,0]) circle(8);
                translate([-33,0,0]) circle(8);
            }
            translate([33,0,0]) circle(6);
            translate([-33,0,0]) circle(6);
        }
        linear_extrude(height=13) difference(){
            union(){
                translate([33,0,0]) circle(8);
                translate([-33,0,0]) circle(8);
            }
            translate([33,0,0]) circle(6);
            translate([-33,0,0]) circle(6);
        } 
    }
}
battery_holder();

module bolts(){
    hg = 8;  // height
    sth = 4; // step height
    clf = 55; // degrees
    cod = 9.5; // outer diameter
    crs = 1; // resolution
    hex_screw(cod,sth,clf,20,1.5,2,15,5,0,0);
}

translate([20,20,0]) bolts();
translate([-20,20,0]) bolts();
