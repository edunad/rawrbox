#include "particles_uniforms.fxh"

#define READ_PARTICLES
#include "particles.fxh"

struct VSInput {
    uint VertexID : SV_VertexID;
};

struct PSInput {
    float4 POS       : SV_POSITION;
    float2 SIZE      : SIZE;
    float3 UV        : TEXCOORD0;
    float4 COLOR     : COLOR;

    uint   TextureID : TEX_ARRAY_INDEX;
};

void main(in VSInput VSIn, out PSInput PSIn) {
    Particle particle = GetParticle(VSIn.VertexID);

    PSIn.POS = float4(particle.position, 1.0);
    PSIn.UV = float3(0.0, 0.0, particle.atlasIndex);
    PSIn.SIZE = particle.size;
    PSIn.COLOR = particle.color;
    PSIn.TextureID = EmitterConstants.textureID;
}
