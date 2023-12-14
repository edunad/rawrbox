
#include <lit_uniforms.fxh>

#define TRANSFORM_DISPLACEMENT
#define TRANSFORM_PSX
#define TRANSFORM_BILLBOARD
#define TEXTURE_DATA

#include <material.fxh>
#include <model_transforms.fxh>

struct VSInput {
    float3 Pos     : ATTRIB0;
    float4 UV      : ATTRIB1;
    float4 Color   : ATTRIB2;
    float3 Normal  : ATTRIB3;
    float3 Tangent : ATTRIB4;
};

struct PSInput {
    float4 Pos                      : SV_POSITION;
    float4 WorldPos                 : POSITION1;

    float3 Normal                   : NORMAL0;
    float3 Tangent                  : TANGENT0;

    float2 UV                       : TEX_COORD;
    float4 Color                    : COLOR0;

    nointerpolation float  TexIndex : TEX_ARRAY_INDEX;
};


void main(in VSInput VSIn, out PSInput PSIn) {
    TransformedData transform = applyPosTransforms(VSIn.Pos, VSIn.UV.xy);

    PSIn.Normal   = normalize(mul(g_Camera.model, float4(VSIn.Normal, 0.0)).xyz);
    PSIn.Tangent  = normalize(mul(g_Camera.model, float4(VSIn.Tangent, 0.0)).xyz);

    PSIn.Pos      = transform.final;
    PSIn.WorldPos = mul(transform.pos, g_Camera.model);
    PSIn.UV       = applyUVTransform(VSIn.UV.xy);
    PSIn.TexIndex = VSIn.UV.z;

    PSIn.Color    = VSIn.Color * g_Model.colorOverride;
}
