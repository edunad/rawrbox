#include "materials/particles/include/particle_vertex.fxh"

void main(in VSInput VSIn, out PSInput PSIn) {
	PSIn = particleVertex(VSIn.VertexID);
}
