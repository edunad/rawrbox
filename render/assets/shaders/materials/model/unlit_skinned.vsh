#define MAX_BONES 200 // TODO: MOVE IT TO MACROS
#define NUM_BONES_PER_VERTEX 4 // TODO: MOVE IT TO MACROS

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

    float4x4 g_bones[MAX_BONES];
    // ------------
};

struct VSInput {
    float3 Pos        : ATTRIB0;
    float4 UV         : ATTRIB1;
    float4 Color      : ATTRIB2;

    uint4 BoneIndex   : ATTRIB3;
    float4 BoneWeight : ATTRIB4;
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
#define TRANSFORM_BONES
#define TEXTURE_DATA
#include "material.fxh"
#include "model_transforms.fxh"

void main(in VSInput VSIn, out PSInput PSIn) {
    float4 pos = boneTransform(VSIn.BoneIndex, VSIn.BoneWeight, VSIn.Pos);
    TransformedData transform = applyPosTransforms(pos, VSIn.UV.xy);

    PSIn.Pos      = transform.final;
    PSIn.UV       = VSIn.UV.xy;
    PSIn.TexIndex = VSIn.UV.z;

    PSIn.Color    = VSIn.Color * g_ColorOverride;
}
