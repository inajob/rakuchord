include <polyScrewThread_r1.scad>;

//linear_extrude(height = 5, center = true, convexity = 10)
$fn=50;

module screw_hole(){
    translate([92/2 - 5, 92/2-2 + 1,0]) screw_hole_model();
    translate([92/2 - 5, -92/2 + 3,0]) screw_hole_model();

    translate([-92/2 + 5, 92/2-2 + 1,0]) screw_hole_model();
    translate([-92/2 + 5, -92/2 + 3,0]) screw_hole_model();
}
module screw_hole_model(){
    union(){
        cylinder(h = 5, r1 = 3.5, r2 = 3.5, center = false, $fn=6);    
        translate([0,0,5])
          cylinder(h = 3, r1 = 3.5, r2 = 2.5, center = false);
        translate([0,0,0])
          cylinder(h = 200, r1 = 2.5, r2 = 2.5);
    }
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

module upper_case_outer(){
    translate([0,0, 6]) rotate([0,180,0]) linear_extrude(6){
        translate([0,3,0])
          offset(2) offset(-2){
            square([93,100+4], center=true);
          }
    }
}

module upper_hole(){
  //square([73, 90], center = true);
  translate([-136,77,0])
    import(file = "dxf/rakuchord-draft-Eco1.User.dxf");
}
module upper_hole2(){
  translate([-136,77,0])
    import(file = "dxf/rakuchord-draft-Dwgs.User.dxf");
}

module key1_space(){
  translate([12+8,100/2 - 1,1]) square([23,3]); // key1
  translate([-6,100/2 - 5,1]) square([25,3]); // key1
}

module cable_hole(){
  translate([-5,-100/2,1]) square([30,5]);
}

module jack_diff(){
    translate([14,-40+3,3]) rotate([90,0,0]) linear_extrude(height=20){
       translate([0,5,0])square([14,14], center = true);
       circle(7);
    }
}

*base_board();

// not maintain: case
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

// no use: upper parts of top
*linear_extrude(height=3) difference(){
    translate([0,0, 0]) rotate([0,180,0]){
        translate([0,3,0])
          offset(2) offset(-2){
            square([93,100+4], center=true);
          }
    }
    translate([92/2 - 5, 92/2-2 + 1,0]) circle(2);
    translate([92/2 - 5, -92/2 + 3,0]) circle(2);
    translate([-92/2 + 5, 92/2-2 + 1,0]) circle(2);
    translate([-92/2 + 5, -92/2 + 3,0]) circle(2);
    upper_hole();
}

// no use: lower parts of top
*translate([0,0,3]) linear_extrude(height=3) difference(){
    translate([0,0, 0]) rotate([0,180,0]){
        translate([0,3,0])
          offset(2) offset(-2){
            square([93,100+4], center=true);
          }
    }

    translate([92/2 - 5, 92/2-2 + 1,0]) circle(2);
    translate([92/2 - 5, -92/2 + 3,0]) circle(2);
    translate([-92/2 + 5, 92/2-2 + 1,0]) circle(2);
    translate([-92/2 + 5, -92/2 + 3,0]) circle(2);

    upper_hole2();
}

// top
*translate([0,0,40]) difference(){
    upper_case_outer();
    translate([0,0,0])linear_extrude(height=3){
      upper_hole();
      *cable_hole();
    }
    translate([0,0,-10])linear_extrude(height=50){
      upper_hole2();
    }
    translate([0,0,-30])
      screw_hole();
    translate([0,0,0]) linear_extrude(height=3){
        //base_board_space();
        *difference(){
            offset(1){
                lower_case_outer();
            }
            offset(0){
                lower_case_inner();
            }
        }
    }
    translate([0,0,2])linear_extrude(height=5){
      *key1_space();
    }
    jack_diff();
    // this makes easier for 3D print
    translate([0,0,5.5]) linear_extrude(height = 0.5){
      translate([20,4,0]) square([10,0.5], center = true);
      
      translate([50,45,0]) square([20,0.5], center = true);
      translate([-50,45,0]) square([20,0.5], center = true);
      translate([50,-43,0]) square([20,0.5], center = true);
      translate([-50,-43,0]) square([20,0.5], center = true);
    }
}

module sub_board(){
      translate([0,100/2 + 2,-15]) rotate([90,0,0]) square([93, 30], center = true);
}

module lower_case_outer(){
    hull(){
      translate([93/2 ,100/2 + 2,0]) circle(5, $fn=50);
      translate([-93/2 ,100/2 + 2,0]) circle(5, $fn=50);
    }
    square([92+1+6, 100+10], center = true);
    hull(){
        translate([93/2 -1 ,-100/2,0]) circle(2, $fn=50);
        translate([-93/2 +1 ,-100/2,0]) circle(2, $fn=50);
    }
}
module lower_case_outer_edge(){
  square([92+1, 105], center = true);
}

module lower_case_inner(){
    square([95 - 6, 105], center = true);
    translate([0,52,0]) square([94, 3], center = true);
    translate([0,50,0]) square([93 - 8, 3], center = true);

    translate([0,55,0]) square([93 - 15+10, 5], center = true);
    translate([0,-50,0]) square([57, 5+10], center = true);
}

*sub_board();

module speaker_hold(){
    linear_extrude(height = 20){
        translate([-94/2 + 12,-50 + 5 + 10/2-5,0]) scale(5) {
            translate([-2,-1,0]) square([1, 3.5]);
            mirror([0,1,0]) polygon([[-1,-2.5],[1,-2.5],[1,0],[0,1],[-1,1]]);
            translate([-2,-2,0]) square([1.5,1]);
        }
        
    }
    linear_extrude(height = 15){
        translate([-94/2 + 12,45.5,0]) scale(5) {
            translate([-2,-1,0]){
                square([1, 2]);
                
            }
            translate([-1,0,0]){
                circle(1, $fn=50);
            }
        }
        translate([0,40,0])
          square([49,5]);
    }
}


thread = "none"; //modeled


*translate([0,0,-34]){
    difference(){
        union(){
            linear_extrude(height = 36){
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
            linear_extrude(height=20)translate([0,-32,0])square([96,3],center=true);
            translate([46+2,-51-5,0]){
                cylinder(h = 11, r1 = 4, r2 = 4, $fn = 40);
                translate([0,0,11])cylinder(h = 5, r1 = 4, r2 = 2, $fn = 40);
                
                translate([0,0,16])cylinder(h = 4, r1 = 2, r2 = 2, $fn = 40);
                
                translate([0,0,20])cylinder(h = 5, r1 = 2, r2 = 4, $fn = 40);
                translate([0,0,20 + 5])cylinder(h = 11, r1 = 4, r2 = 4, $fn = 40);
            }
            translate([49,57,0]){
                cylinder(h = 11, r1 = 4, r2 = 4, $fn = 40);
                translate([0,0,11])cylinder(h = 5, r1 = 4, r2 = 2, $fn = 40);
                
                translate([0,0,16])cylinder(h = 4, r1 = 2, r2 = 2, $fn = 40);
                
                translate([0,0,20])cylinder(h = 5, r1 = 2, r2 = 4, $fn = 40);
                translate([0,0,20 + 5])cylinder(h = 11, r1 = 4, r2 = 4, $fn = 40);
            }
        }
        translate([0,0,33]){
            linear_extrude(height = 5){
                lower_case_outer_edge();
            }
        }
        screw_hole();
        // this makes easier for 3D print
        linear_extrude(height = 0.5){
          translate([50,0,0]) square([20,0.5], center = true);
          translate([-50,0,0]) square([20,0.5], center = true);
          translate([50,45,0]) square([20,0.5], center = true);
          translate([-50,45,0]) square([20,0.5], center = true);
          translate([50,-43,0]) square([20,0.5], center = true);
          translate([-50,-43,0]) square([20,0.5], center = true);
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

*translate([20,20,0]) bolts();
*translate([-20,20,0]) bolts();
