

//###### main dimensions

//The width and depth of the inner cavity of the torch
PL_INNER = 70;

//The main support beams are square prisms. This is their width and depth.
BEAM_SIDE_LENGTH = 10;

//Support channels for beams
SUPPORTS_CLEARANCE = 0.5;

//The width or depth of a channel for a support beam
BEAM_CHANNEL_DIAMETER = BEAM_SIDE_LENGTH + SUPPORTS_CLEARANCE;

//The width or depth of a channel that has a half-height support beam
HALF_BEAM_CHANNEL_DIAMETER = BEAM_SIDE_LENGTH/2 + SUPPORTS_CLEARANCE;

//the outer width and depth of the torch (except for the light emitting end)
// This also doesn't take into account the thickness of the panels being mounted to the beams.
PL_OUTER = PL_INNER+2*BEAM_SIDE_LENGTH;

//The width of side panels.
SP_WIDTH = 90;

//Fixing holes positions
cpfh_y1 = BEAM_CHANNEL_DIAMETER/2;
cpfh_y2 = PL_OUTER-BEAM_CHANNEL_DIAMETER/2;

cpfh_x1 = 15;
cpfh_x2 = 25;



//###### Screw holes bits
m3_tight_diameter = 2.8;


$fn = 40;
layer_height = 0.2;
