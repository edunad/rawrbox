
#include <lit_uniforms.fxh>
#include <model_transforms.fxh>

struct VSInput {
    float3 Pos   : ATTRIB0;
    float4 UV    : ATTRIB1;
    float4 Color : ATTRIB2;
};

struct PSInput {
    float4 Pos                      : SV_POSITION;
    //float4 WorldPos                 : POSITION1;
    float2 UV                       : TEX_COORD;
    float4 Color                    : COLOR0;

    nointerpolation float  TexIndex : TEX_ARRAY_INDEX;
};

void main(in VSInput VSIn, out PSInput PSIn) {
    TransformedData transform = applyPosTransforms(VSIn.Pos, VSIn.UV.xy);

    PSIn.Pos      = transform.final;
    //PSIn.WorldPos = mul(transform.pos, g_Camera.model);
    PSIn.UV       = VSIn.UV.xy;
    PSIn.TexIndex = VSIn.UV.z;

    PSIn.Color    = VSIn.Color * g_Model.colorOverride;
}
