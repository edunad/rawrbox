#include <camera.fxh>

struct ConstantsStruct {
    float4 g_Billboard;
};

ConstantBuffer<ConstantsStruct> Constants;

#define Billboard Constants.g_Billboard

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
