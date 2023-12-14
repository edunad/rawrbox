
#include <unlit_uniforms.fxh>

#define TEXTURE_DATA
#include <material.fxh>

#define TRANSFORM_DISPLACEMENT
#define TRANSFORM_PSX
#define TRANSFORM_BILLBOARD
#include <model_transforms.fxh>

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

void main(in VSInput VSIn, out PSInput PSIn) {
    float4x4 InstanceMatr = MatrixFromRows(VSIn.MtrxRow0, VSIn.MtrxRow1, VSIn.MtrxRow2, VSIn.MtrxRow3);
    TransformedData transform = applyPosTransforms(g_Camera.viewProj, mul(float4(VSIn.Pos, 1.), InstanceMatr), VSIn.UV);

    PSIn.Pos      = transform.final;
    PSIn.Color    = VSIn.Color * VSIn.ColorOverride * g_Model.colorOverride;
    PSIn.UV       = VSIn.UV;
    PSIn.TexIndex = VSIn.Extra.x;
}
