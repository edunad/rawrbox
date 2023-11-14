
#include <unlit_uniforms.fxh>

#define TEXTURE_DATA
#include <material.fxh>

#define TRANSFORM_DISPLACEMENT
#define TRANSFORM_PSX
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

    nointerpolation float  TexIndex : TEX_ARRAY_INDEX;
};


void main(in VSInput VSIn, out PSInput PSIn) {
    TransformedData transform = applyPosTransforms(VSIn.Pos, VSIn.UV.xy);

    PSIn.Pos      = transform.final;
    PSIn.UV       = applyUVTransform(VSIn.UV.xy);
    PSIn.TexIndex = VSIn.UV.z;
    PSIn.Color    = VSIn.Color * g_Model.colorOverride;
}
