cbuffer Constants {
    // GLOBAL ----
    float4x4 g_Model;
    float4x4 g_ViewProj;
    float4x4 g_InvView;
    float4x4 g_WorldViewProj;
    float4   g_ScreenSize;
    // ------------

    // UNIFORMS ----
    float4   g_ColorOverride;
    float4   g_TextureFlags;
    float4   g_Data[4];
    // ------------
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

    nointerpolation float  TexIndex : TEX_ARRAY_INDEX;
};

#define TRANSFORM_DISPLACEMENT
#define TRANSFORM_PSX
#define TRANSFORM_BILLBOARD
#define TEXTURE_DATA
#include "material.fxh"
#include "model_transforms.fxh"

void main(in VSInput VSIn, out PSInput PSIn) {
    TransformedData transform = applyPosTransforms(VSIn.Pos, VSIn.UV.xy);
    //v_worldPos = mul(u_model[0], transform.pos).xyz;
    PSIn.Pos = transform.final;
    PSIn.UV       = VSIn.UV.xy;
    PSIn.TexIndex = VSIn.UV.z;

    PSIn.Color    = VSIn.Color * g_ColorOverride;
}
