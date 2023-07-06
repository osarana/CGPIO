# CGPIO
# Program 7.3 - Torus with Blinn-Phong Shading

Although Phong shading offers better realism than Gouraud shading, it does so while incurring a performance cost. One optimization to Phong shading was proposed by James Blinn in 1977, and is referred to as the Blinn-Phong reflection model.

It is based on the observation that one of the most expensive computations in Phong shading is determing the reflection vector R.

# Reflection Vector in Performance
Blinn observed that the vector R itself is not needed as R is only produced as a means of determining the angle Φ. It turns out that Φ can be found without computing R, by instead computing a vector H that is halfway between L and V. 

The halfway vector H is most easily determined by finding L + V, after which its cos(a) can be found using the dot product H * N.

# Suggestions
The computations can be done in the fragment shader, or even in the vertex shader (with some tweaks) if necessary for performance.
