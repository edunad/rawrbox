#include "particles_uniforms.fxh"
#include "camera.fxh"

#define READ_PARTICLES
#include "particles.fxh"

struct VSInput {
    uint VertexID : SV_VertexID;
};

struct PSInput {
    float4 POS    : SV_POSITION;
    float2 UV     : TEXCOORD0;
    float4 COLOR  : COLOR;
};

void main(in VSInput VSIn, out PSInput PSIn) {
    // Declare quad vertices
    float4 Pos[4];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4(-0.5, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, +0.5, 0.0, 1.0);
    Pos[3] = float4(+0.5, -0.5, 0.0, 1.0);

    // Declare quad indices
    uint Indices[6] = { 0, 1, 2, 0, 2, 3 };

    // Get particle index and vertex index
    uint particleIndex = VSIn.VertexID / 6;
    uint vertexIndex = Indices[VSIn.VertexID % 6];

    // Get particle data
    Particle particle = GetParticle(particleIndex);

    // Set output position based on particle position and quad vertex position
    float4 worldPos = float4(particle.position, 1.0) + float4(Pos[vertexIndex].xy * particle.size, 0.0, 0.0);
    PSIn.POS = mul(worldPos, Camera.worldViewProj);

    // Set output UV based on quad vertex
    float2 UV[4];
    UV[0] = float2(0.0, 1.0);
    UV[1] = float2(0.0, 0.0);
    UV[2] = float2(1.0, 0.0);
    UV[3] = float2(1.0, 1.0);
    PSIn.UV = UV[vertexIndex];

    // Set output color based on particle color
    PSIn.COLOR = particle.color;
}
