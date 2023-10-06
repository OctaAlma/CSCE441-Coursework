Octavio Almanza
628005492
oa5967@tamu.edu

I completed up to Task 6 and I implemented the 5 point bonus for window resizing.

I did not utilize any code from the internet.

To generate the four textures required in task 6, go to the pass2_frag.glsl file and uncomment:
 - Line 59 for blinn-phong texture
 - Line 60 for camera-space position texture
 - Line 61 for camera-space normal texture
 - Line 62 for emissive color texture
 - Line 63 for diffuse color texture

I have two vertex shaders and two fragment shaders.
I perform the computations for the solid of revolution in pass1_vert.glsl whenever I pass a "1" to the uniform float "isSor".