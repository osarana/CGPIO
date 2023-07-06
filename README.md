# CGPIO
# Program 7.1 - Torus with Positional Light and Gouraud Shading

The French computer scientist Henri Gouraud published a smooth shading algorithm in 1971, known as Gouraud shading. It is well suited to modern graphics cards because it takes advantage of the automatic interpolated rendering available in 3D graphics pipelines such as OpenGL.

# Gouraud Shading Process
* Determine the color of each vertex, incorporating the lighting computations.
* Allow those colors to be interpolated across intervening pixels through the normal rasterization process (which will also interpolate the lighting contributions).

In OpenGL, this means that most of the lighting computations will be done in the vertex shader. The fragment shader will simply be a pass-through, so as to reveal the automatically interpolated lighted colors.
