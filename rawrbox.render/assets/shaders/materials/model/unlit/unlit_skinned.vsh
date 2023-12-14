
#include <unlit_skinned_uniforms.fxh>

#define TEXTURE_DATA
#include <material.fxh>

#define TRANSFORM_DISPLACEMENT
#define TRANSFORM_PSX
#define TRANSFORM_BILLBOARD
#define TRANSFORM_BONES
#include <model_transforms.fxh>

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

void main(in VSInput VSIn, out PSInput PSIn) {
    float4 pos = boneTransform(VSIn.BoneIndex, VSIn.BoneWeight, VSIn.Pos);
    TransformedData transform = applyPosTransforms(pos, VSIn.UV.xy);

    PSIn.Pos      = transform.final;
    PSIn.UV       = VSIn.UV.xy;
    PSIn.TexIndex = VSIn.UV.z;
    PSIn.Color    = VSIn.Color * g_Model.colorOverride;
}
