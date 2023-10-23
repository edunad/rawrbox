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
    float2 UV    : ATTRIB1;
    float4 Color : ATTRIB2;

    // Instance attributes
    float4 MtrxRow0      : ATTRIB3;
    float4 MtrxRow1      : ATTRIB4;
    float4 MtrxRow2      : ATTRIB5;
    float4 MtrxRow3      : ATTRIB6;
    float4 ColorOverride : ATTRIB7;
    float4 Extra         : ATTRIB8;
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
    float4x4 InstanceMatr = MatrixFromRows(VSIn.MtrxRow0, VSIn.MtrxRow1, VSIn.MtrxRow2, VSIn.MtrxRow3);
    TransformedData transform = applyPosTransforms(g_ViewProj, mul(float4(VSIn.Pos, 1.), InstanceMatr), VSIn.UV);
    //v_worldPos = mul(u_model[0], transform.pos).xyz;
    PSIn.Pos = transform.final;

    PSIn.Color    = VSIn.Color * VSIn.ColorOverride * g_ColorOverride;

    PSIn.UV       = VSIn.UV;
    PSIn.TexIndex = VSIn.Extra.x;
}
