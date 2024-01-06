
#include <uniforms.fxh>

#define TEXTURE_DATA
#include <material.fxh>

//#define TRANSFORM_DISPLACEMENT
#define TRANSFORM_PSX
#define TRANSFORM_BILLBOARD
#ifdef SKINNED
    #define TRANSFORM_BONES
#endif

#include <model_transforms.fxh>

struct VSInput {
    float3 Pos   : ATTRIB0;

    float4 UV    : ATTRIB1;
    float4 Color : ATTRIB2;

    #ifdef SKINNED
        uint4 BoneIndex   : ATTRIB3;
        float4 BoneWeight : ATTRIB4;

        #ifdef INSTANCED
            // Instance attributes
            float4 MtrxRow0      : ATTRIB5;
            float4 MtrxRow1      : ATTRIB6;
            float4 MtrxRow2      : ATTRIB7;
            float4 MtrxRow3      : ATTRIB8;
            float4 ColorOverride : ATTRIB9;
            float4 Extra         : ATTRIB10;
        #endif
    #else
        #ifdef INSTANCED
            // Instance attributes
            float4 MtrxRow0      : ATTRIB3;
            float4 MtrxRow1      : ATTRIB4;
            float4 MtrxRow2      : ATTRIB5;
            float4 MtrxRow3      : ATTRIB6;
            float4 ColorOverride : ATTRIB7;
            float4 Extra         : ATTRIB8;
        #endif
    #endif
};

struct PSInput {
    float4 Pos                      : SV_POSITION;
    float4 WorldPos                 : POSITION1;

    float2 UV                       : TEX_COORD;
    float4 Color                    : COLOR0;

    nointerpolation uint   TexIndex : TEX_ARRAY_INDEX;
};


void main(in VSInput VSIn, out PSInput PSIn) {
    #ifdef SKINNED
        float4 pos = boneTransform(VSIn.BoneIndex, VSIn.BoneWeight, VSIn.Pos);
    #else
        float4 pos = float4(VSIn.Pos, 1.);
    #endif

    #ifdef INSTANCED
        float4x4 InstanceMatr = MatrixFromRows(VSIn.MtrxRow0, VSIn.MtrxRow1, VSIn.MtrxRow2, VSIn.MtrxRow3);
        TransformedData transform = applyPosTransforms(mul(pos, InstanceMatr), VSIn.UV.xy);
    #else
        TransformedData transform = applyPosTransforms(pos, VSIn.UV.xy);
    #endif

    PSIn.Pos          = transform.final;
    PSIn.WorldPos     = mul(transform.pos, Camera.world);
    PSIn.UV           = applyUVTransform(VSIn.UV.xy);

    #ifdef INSTANCED
        PSIn.Color    = VSIn.Color * VSIn.ColorOverride * Constants.model.colorOverride;
        PSIn.TexIndex = VSIn.Extra.x;
    #else
        PSIn.Color    = VSIn.Color * Constants.model.colorOverride;
        PSIn.TexIndex = VSIn.UV.z;
    #endif
}
