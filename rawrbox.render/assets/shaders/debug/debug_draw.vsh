#include "camera.fxh"
#include "unpack.fxh"

struct VSInput {
    float3 Pos : ATTRIB0;
    uint Color : ATTRIB1; // RGBA8
};

struct PSInput {
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

void main(in VSInput VSIn, out PSInput PSIn) {
    PSIn.Pos = mul(float4(VSIn.Pos, 1.0), Camera.worldViewProj);
    PSIn.Color = Unpack_RGBA8_UNORM(VSIn.Color);
}
