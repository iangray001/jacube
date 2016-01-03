// Rounded primitives for openscad
// (c) 2013 Wouter Robers 

// Syntax example for a rounded block
//translate([-15,0,10]) rcube([20,20,20],2);

// Syntax example for a rounded cylinder
//translate([15,0,10]) rcylinder(r1=15,r2=10,h=20,b=2);

$fn=20;


//gp: non-offset flat bottomed rounded cube:
module nfrcube(Size, b=2) {
        //cube([1,1,Size[2]]);
        translate([Size[0]/2,Size[1]/2,Size[2]/2]) 
        
        difference() {
                translate([0,0,-b/2]) rcube([Size[0],Size[1],Size[2]+b],b);
                translate([0,0,-Size[2]/2-b/2])
                        cube([Size[0],Size[1],b], center=true);
        }
}

module rcube(Size=[20,20,20],b=2)
{hull(){for(x=[-(Size[0]/2-b),(Size[0]/2-b)]){for(y=[-(Size[1]/2-b),(Size[1]/2-b)]){for(z=[-(Size[2]/2-b),(Size[2]/2-b)]){ translate([x,y,z]) sphere(b);}}}}}


module rcylinder(r1=10,r2=10,h=10,b=2)
{translate([0,0,-h/2]) hull(){rotate_extrude() translate([r1-b,b,0]) circle(r = b); rotate_extrude() translate([r2-b, h-b, 0]) circle(r = b);}}