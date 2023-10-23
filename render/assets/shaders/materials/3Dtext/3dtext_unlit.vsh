
cbuffer Constants {
    // GLOBAL ----
    float4x4 g_WorldViewProj;
    float4x4 g_InvView;
    // ------------

    float4 g_Billboard;
};

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


#define TRANSFORM_BILLBOARD
#include "model_transforms.fxh"

void main(in VSInput VSIn, out PSInput PSIn) {
    TransformedData transform = applyPosTransforms(VSIn.Pos, VSIn.UV.xy);
    PSIn.Pos = transform.final;

    PSIn.UV       = VSIn.UV.xy;
    PSIn.Color    = VSIn.Color;
}
