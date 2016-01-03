/*===========================================================================================
Filename: jacube_assembly.scad
 
Description: All parts in their positions in the jacube to highlight assembly.

Author: Gary Plumbridge

=========================================================================================*/

include <configuration.scad>
include <main_dimensions.scad>
include <jacube_bits.scad>


//beams
translate([0,BEAM_SIDE_LENGTH,PANEL_FLANGE_WIDTH])
        rotate([0,0,270])
                supportbeam();

translate([0,JACUBE_FLANGELESS_WIDTH,PANEL_FLANGE_WIDTH])
        rotate([0,0,270])
                supportbeam();

//Panels
union() {
    
    //bottom
    sidepanel(true, false);
    
    
    
    //sides
    union() {
        translate([0,JACUBE_FLANGELESS_WIDTH+PANEL_FLANGE_WIDTH ,PANEL_FLANGE_WIDTH])
                rotate([90,0,0])
                        sidepanel(false, true);
        
        translate([0,-PANEL_FLANGE_WIDTH,JACUBE_FLANGELESS_WIDTH+PANEL_FLANGE_WIDTH])
                rotate([270,0,0])
                        sidepanel(false, true);
        
        translate([JACUBE_FLANGELESS_WIDTH+PANEL_FLANGE_WIDTH,0,PANEL_FLANGE_WIDTH])
                rotate([0,270,0])
                        sidepanel(false); 
                    
        translate([-PANEL_FLANGE_WIDTH,0,JACUBE_FLANGELESS_WIDTH+PANEL_FLANGE_WIDTH])
                rotate([0,90,0])
                        $sidepanel(false);     
    }         
    
    //Lid
    translate([0,JACUBE_FLANGELESS_WIDTH,JACUBE_FLANGELESS_WIDTH+PANEL_FLANGE_WIDTH*2])
            rotate([180,0,0])
                    sidepanel(true, false);
                                
}

//exports:

//a top panel
sidepanel(true, false);

//side with inner screws
sidepanel(false, true);

//side with outer screws
sidepanel(false, false);

//A battery brace
translate([20,11,11]) 
slimbrace();
