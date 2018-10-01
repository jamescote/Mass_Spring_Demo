Compiling on Windows:
Compiled using MSVS 2015.
You'll need to set up OpenGL, GLEW, and trimesh.lib in the settings of the solution as required libraries.

Compiling on Linux:
In the file EnvSpec.h: You'll need to switch the comment from Defining Windows to Defining Linux.
The included Makefile should generate a run file that will launch the program.  Ensure that this is executed through the command line in order to provide command input functionality of the program.

Controls:
WASD - controls the position of the light along the XZ-axis
Space and X - moves the light along the Y-axis; up and down respectively
Mouse Wheel - Zoom
Right-Click + Drag - Rotate around look-at position
P - Pauses animation (toggle)
Up-Key - Updates 1 Frame, useful for frame stepping when paused.

Scene Modification:
The default scene that gets loaded is "scene2.txt". The Mass Spring is loaded as so:

mass_spring { 
	length depth height 
	k restLength mass 
	damping_coeff delta_t update_loop_count 
	Collision_K Collision_Damping_Coeff 
	type:{cube, cloth, spring, chain, flag}
}

Play around with it for some different evaluations. The types only work to fix certain points:

Spring/Chain -> Fixes the first mass.
cloth -> Fixes the first mass and the iLength mass
cube -> No masses are fixed
flag -> All first masses along the height are fixed. Flag functionality isn't implemented.

Collision works with the {0,0,0} plane. Did not get collision working with other objects.
