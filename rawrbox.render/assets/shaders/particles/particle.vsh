#include "particles_uniforms.fxh"
#include "camera.fxh"

#define READ_PARTICLES
#include "particles.fxh"


struct VSInput {
    uint VertexID : SV_VertexID;
};

struct PSInput {
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD0;
    float3 Color    : COLOR;
};

void main(in VSInput VSIn, out PSInput PSIn) {
    // Declare quad vertices
    float4 Pos[4];
    Pos[0] = float4(-1.0, -1.0, 0.0, 1.0);
    Pos[1] = float4(-1.0, +1.0, 0.0, 1.0);
    Pos[2] = float4(+1.0, -1.0, 0.0, 1.0);
    Pos[3] = float4(+1.0, +1.0, 0.0, 1.0);

    // Declare quad UVs
    float2 UV[4];
    UV[0] = float2(+0.0, +1.0);
    UV[1] = float2(+0.0, +0.0);
    UV[2] = float2(+1.0, +1.0);
    UV[3] = float2(+1.0, +0.0);

    // Get particle data
    Particle particle = GetParticle(VSIn.VertexID / 4);

    // Set output position and UV based on quad vertex
    PSIn.Position = mul(Pos[VSIn.VertexID % 4], Camera.view);
    PSIn.Position.xy += particle.position.xy;
    PSIn.UV = UV[VSIn.VertexID % 4];

    // Set output color based on particle lifetime
    PSIn.Color = lerp(float3(1.0, 0.0, 0.0), float3(1.0, 1.0, 0.0), particle.data.x / EmitterConstants.lifetime);
}
