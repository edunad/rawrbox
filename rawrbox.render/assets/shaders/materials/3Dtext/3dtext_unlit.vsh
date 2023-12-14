
struct Camera {
    float4x4 worldViewProj;
    float4x4 invView;
};


cbuffer Constants {
    Camera g_Camera;
    float4 g_Billboard;
};

#define Billboard g_Billboard

#define TRANSFORM_BILLBOARD
#include <model_transforms.fxh>

struct VSInput {
    float3 Pos   : ATTRIB0;
    float4 UV    : ATTRIB1;
    float4 Color : ATTRIB2;
};

struct PSInput {
    float4 Pos                      : SV_POSITION;
    float2 UV                       : TEX_COORD;
    float4 Color                    : COLOR0;
};

void main(in VSInput VSIn, out PSInput PSIn) {
    TransformedData transform = applyPosTransforms(VSIn.Pos, VSIn.UV.xy);
    PSIn.Pos = transform.final;

    PSIn.UV       = VSIn.UV.xy;
    PSIn.Color    = VSIn.Color;
}
