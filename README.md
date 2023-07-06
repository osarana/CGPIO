# CGPIO
# Program 7.2 - Torus with Phong Shading

Bui Tuong Phong developed a smooth shading algorithm while a graduate student at the University of Utah. The structure of the algorithm is similar to the algorithm for Gouraud shading, except that the lighting computations are done per pixel rather than per vertex. Since the lighting computations require a normal vector N and light vector L, which are only available in the model on a per-vertex basis, Phong shading is often implemented using a clever "trick" whereby N and L are computed in the vertex shader and interpolated during rasterization.

Phong shading corrects the artifacts present in Gouraud shading.
