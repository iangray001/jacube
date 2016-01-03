/*===========================================================================================
Filename: jacube_bits.scad
 
Description: Parts (panels and beams) for the jacube

Author: Gary Plumbridge

=========================================================================================*/

include <rcube.scad>
include <configuration.scad>
include <main_dimensions.scad>

JACUBE_EXT_LENGTH = 100;
PANEL_FLANGE_WIDTH = BEAM_SIDE_LENGTH/2;
JACUBE_FLANGELESS_WIDTH = JACUBE_EXT_LENGTH-PANEL_FLANGE_WIDTH*2;
INNER_WIDTH = JACUBE_FLANGELESS_WIDTH-2*BEAM_SIDE_LENGTH-1;

CLEARANCE = 0.5;
BEAM_LENGTH = JACUBE_FLANGELESS_WIDTH-2*CLEARANCE;

module supportbeam() {

    translate([0,CLEARANCE/2, CLEARANCE/2])    
        innerbeam();
        
    translate([0,CLEARANCE/2, JACUBE_FLANGELESS_WIDTH-BEAM_SIDE_LENGTH-CLEARANCE/2])    
        innerbeam();
    
    translate([0,BEAM_SIDE_LENGTH-CLEARANCE/2,CLEARANCE/2])
        rotate([90,0,0])
            innerbeam();
    
    translate([0,BEAM_LENGTH+CLEARANCE/2,CLEARANCE/2])
        rotate([90,0,0])
            innerbeam();
        

}

module innerbeam() {
        

        difference() {
                union() {
                        cube([BEAM_SIDE_LENGTH-CLEARANCE,BEAM_LENGTH,BEAM_SIDE_LENGTH-CLEARANCE]);
                }             
                    
                //Screw holes
                        
                    //top/bottom panels
                    translate([BEAM_SIDE_LENGTH/2-CLEARANCE/2, cpfh_x1, -5])
                        cylinder(r=m3_tight_diameter/2, h=BEAM_SIDE_LENGTH*2);
                    translate([BEAM_SIDE_LENGTH/2-CLEARANCE/2, BEAM_LENGTH-cpfh_x1+CLEARANCE, -5])
                        cylinder(r=m3_tight_diameter/2, h=BEAM_SIDE_LENGTH*2);
                        
                    //side panels
                    translate([-5, cpfh_x2, BEAM_SIDE_LENGTH/2-CLEARANCE/2])
                        rotate([0,90,0])
                                cylinder(r=m3_tight_diameter/2, h=BEAM_SIDE_LENGTH*2);
                    translate([-5, BEAM_LENGTH-cpfh_x2+CLEARANCE, BEAM_SIDE_LENGTH/2-CLEARANCE/2])
                        rotate([0,90,0])
                                cylinder(r=m3_tight_diameter/2, h=BEAM_SIDE_LENGTH*2);
                             
                
                
        }
}

/*
Makes a brace that screws onto an innerscrew sidepanel to hold the battery pack in place.
*/
module brace() {
        
        washer=10;

        difference() {
            nfrcube([20, INNER_WIDTH, 20], 5);
            
            //Washer/screw channels
            translate([0, INNER_WIDTH/2, 5]) {
                translate([0,-washer/2,0]) nfrcube([15,washer,20]);
                translate([0,-m3_diameter/2,-10]) nfrcube([10,m3_diameter,20]);
            }
                
            translate([0, INNER_WIDTH/2-JACUBE_FLANGELESS_WIDTH*0.25, 5]) {
                translate([0,-washer/2,0]) nfrcube([15,washer,20]);
                translate([0,-m3_diameter/2,-10]) nfrcube([10,m3_diameter,20]);
            }  
            
            translate([0, INNER_WIDTH/2+JACUBE_FLANGELESS_WIDTH*0.25, 5]) {
                translate([0,-washer/2,0]) nfrcube([15,washer,20]);
                translate([0,-m3_diameter/2,-10]) nfrcube([10,m3_diameter,20]);
            }
        
        }
}

/*
Makes a brace that screws onto an innerscrew sidepanel to hold the battery pack in place.
*/
module slimbrace() {
        
        washer=10;

        difference() {
            nfrcube([20, INNER_WIDTH, 20], 5);
            
            //Washer/screw channels
            translate([0, INNER_WIDTH/2, 5]) {
                translate([0,-washer/2,0]) nfrcube([15,washer,20]);
                translate([0,-m3_diameter/2,-10]) nfrcube([10,m3_diameter,20]);
            }
                
            translate([0, INNER_WIDTH/2-JACUBE_FLANGELESS_WIDTH*0.25, 5]) {
                translate([0,-washer/2,0]) nfrcube([15,washer,20]);
                translate([0,-m3_diameter/2,-10]) nfrcube([10,m3_diameter,20]);
            }  
            
            translate([0, INNER_WIDTH/2+JACUBE_FLANGELESS_WIDTH*0.25, 5]) {
                translate([0,-washer/2,0]) nfrcube([15,washer,20]);
                translate([0,-m3_diameter/2,-10]) nfrcube([10,m3_diameter,20]);
            }
            
            translate([5,0,5])
                cube([20, INNER_WIDTH, 20]);
        
        }
}

/*
makes a side panel for the cube.
@param toporbottom when true this will be a panel for the top or bottom of the cube
@param innerscrewholes when true this panel will have screw holes in the inner positions. Top and bottom panels should use outer positions.
*/
module sidepanel(toporbottom, innerscrewholes) {
        	
        width = JACUBE_FLANGELESS_WIDTH;
        
        FLANGE_LENGTH = (toporbottom ? (width/2)-1+PANEL_FLANGE_WIDTH : (width/2-1));
        FLANGE_OFFSET = (toporbottom ? PANEL_FLANGE_WIDTH : 0);
        
	difference() {
	        union() {
                    //Main chunk (minus a millimetre of clearance)
                    cube([JACUBE_FLANGELESS_WIDTH-0.5, JACUBE_FLANGELESS_WIDTH-0.5, PANEL_FLANGE_WIDTH]);
                    
                    //Strengthener
                    translate([BEAM_SIDE_LENGTH+0.5,BEAM_SIDE_LENGTH+0.5,PANEL_FLANGE_WIDTH])
		        cube([INNER_WIDTH, INNER_WIDTH, 3]);
                    
		    difference() {
		        translate([width/2+0.5,-PANEL_FLANGE_WIDTH,0])    
                            cube([FLANGE_LENGTH, PANEL_FLANGE_WIDTH, PANEL_FLANGE_WIDTH-0.5]);
                        translate([width/2+0.5+FLANGE_LENGTH-PANEL_FLANGE_WIDTH,-1,0])
                            cube([FLANGE_OFFSET, PANEL_FLANGE_WIDTH, PANEL_FLANGE_WIDTH-0.5]);
                    }
                        
                    difference() {
                        translate([-FLANGE_OFFSET,width-0.5,0])
                            cube([FLANGE_LENGTH, PANEL_FLANGE_WIDTH, PANEL_FLANGE_WIDTH-0.5]);
                        translate([-FLANGE_OFFSET,width-0.5,0])
                            cube([FLANGE_OFFSET, PANEL_FLANGE_WIDTH-4, PANEL_FLANGE_WIDTH-0.5]);
                    }
                       
                    difference() {
                        translate([width-0.5,width/2+0.5,0])
                            cube([PANEL_FLANGE_WIDTH, FLANGE_LENGTH , PANEL_FLANGE_WIDTH-0.5]);
                        translate([width-4.5,width-0.5,0])
                            cube([FLANGE_OFFSET, PANEL_FLANGE_WIDTH, PANEL_FLANGE_WIDTH-0.5]);
                    }
                        
                    difference() {
                        translate([-PANEL_FLANGE_WIDTH,-FLANGE_OFFSET,0])
                                cube([PANEL_FLANGE_WIDTH, FLANGE_LENGTH , PANEL_FLANGE_WIDTH-0.5]);
                        translate([-1,-FLANGE_OFFSET,0])
                            cube([FLANGE_OFFSET, PANEL_FLANGE_WIDTH, PANEL_FLANGE_WIDTH-0.5]);
                    }
		}
		
		//LED cutout
		translate([JACUBE_FLANGELESS_WIDTH/2-0.25, JACUBE_FLANGELESS_WIDTH/2-0.25, 1]) 
		        cylinder(r=3,h=50);
			//cube([8,8,50]);
			
		if (innerscrewholes) {
                    //battery brace holes
                    translate([JACUBE_FLANGELESS_WIDTH/2-16, JACUBE_FLANGELESS_WIDTH*0.25, 5]) 
                            cylinder(r=m3_tight_diameter/2,h=50);
                    translate([JACUBE_FLANGELESS_WIDTH/2-16, JACUBE_FLANGELESS_WIDTH*0.5, 5]) 
                            cylinder(r=m3_tight_diameter/2,h=50);
                    translate([JACUBE_FLANGELESS_WIDTH/2-16, JACUBE_FLANGELESS_WIDTH*0.75, 5]) 
                            cylinder(r=m3_tight_diameter/2,h=50);
                    translate([JACUBE_FLANGELESS_WIDTH/2+16, JACUBE_FLANGELESS_WIDTH*0.25, 5]) 
                            cylinder(r=m3_tight_diameter/2,h=50);
                    translate([JACUBE_FLANGELESS_WIDTH/2+16, JACUBE_FLANGELESS_WIDTH*0.5, 5]) 
                            cylinder(r=m3_tight_diameter/2,h=50);
                    translate([JACUBE_FLANGELESS_WIDTH/2+16, JACUBE_FLANGELESS_WIDTH*0.75, 5]) 
                            cylinder(r=m3_tight_diameter/2,h=50);
		}
		
		
		
		//Attachment screw channels (in mounting grooves)
		// There are two sets of mounting holes as the side panes can't use the same holes as
		// the top/bottom panels because the screws would collide.
		union() {
			
			
			if (innerscrewholes) {
                            translate([cpfh_x2,cpfh_y1,0]) cylinder(r=m3_diameter/2, h=BEAM_SIDE_LENGTH/2);
                            translate([cpfh_x2,cpfh_y2,0]) cylinder(r=m3_diameter/2, h=BEAM_SIDE_LENGTH/2);
                            translate([width-cpfh_x2,cpfh_y1,0]) cylinder(r=m3_diameter/2, h=BEAM_SIDE_LENGTH/2);
                            translate([width-cpfh_x2,cpfh_y2,0]) cylinder(r=m3_diameter/2, h=BEAM_SIDE_LENGTH/2);
			
			} else {
			        translate([cpfh_x1,cpfh_y1,0]) cylinder(r=m3_diameter/2, h=BEAM_SIDE_LENGTH/2);
			        translate([cpfh_x1,cpfh_y2,0]) cylinder(r=m3_diameter/2, h=BEAM_SIDE_LENGTH/2);
			        translate([width-cpfh_x1,cpfh_y1,0]) cylinder(r=m3_diameter/2, h=BEAM_SIDE_LENGTH/2);
			        translate([width-cpfh_x1,cpfh_y2,0]) cylinder(r=m3_diameter/2, h=BEAM_SIDE_LENGTH/2);
			}
		}
		
	}
}
