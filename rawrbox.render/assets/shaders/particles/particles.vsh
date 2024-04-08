#define READ_PARTICLES
#include "particles.fxh"

struct VSInput {
    uint VertexID : SV_VertexID;
};

struct PSInput {
    float4 POS         : SV_POSITION;
    float2 SIZE        : SIZE;
    float3x3 ROTATION  : ROTATION;
    float3 UV          : TEXCOORD0;
    float4 COLOR       : COLOR;
};

float3x3 makeRotationMatrix(float3 rotation) {
    // Assuming rotation is in radians and in the order of pitch (X), yaw (Y), roll (Z)
    float cosX = cos(rotation.x);
    float sinX = sin(rotation.x);
    float cosY = cos(rotation.y);
    float sinY = sin(rotation.y);
    float cosZ = cos(rotation.z);
    float sinZ = sin(rotation.z);

    float3x3 rotX = float3x3(
        1, 0, 0,
        0, cosX, -sinX,
        0, sinX, cosX
    );

    float3x3 rotY = float3x3(
        cosY, 0, sinY,
        0, 1, 0,
        -sinY, 0, cosY
    );

    float3x3 rotZ = float3x3(
        cosZ, -sinZ, 0,
        sinZ, cosZ, 0,
        0, 0, 1
    );

    // Combine rotations, order matters, here ZYX
    return mul(mul(rotZ, rotY), rotX);
}


void main(in VSInput VSIn, out PSInput PSIn) {
    Particle particle = GetParticle(VSIn.VertexID);

    PSIn.POS = float4(particle.position, 1.0);
    PSIn.UV = float3(0.0, 0.0, particle.atlasIndex);
    PSIn.SIZE = particle.size;
    PSIn.ROTATION = makeRotationMatrix(particle.rotation);
    PSIn.COLOR = particle.color;
}
