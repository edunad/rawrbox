#include "vertex_bindless_uniforms.fxh"
#include "camera.fxh"

#define TRANSFORM_DISPLACEMENT
#define TRANSFORM_PSX
#define TRANSFORM_BILLBOARD
#ifdef SKINNED
    #define TRANSFORM_BONES
#endif

Texture2DArray g_Textures[];
SamplerState   g_Sampler;

#include "model_transforms.fxh"

struct VSInput {
    float4 Pos     : ATTRIB0;
    float4 UV      : ATTRIB1;

    float4 Normal  : ATTRIB2;
    float4 Tangent : ATTRIB3;

    #ifdef SKINNED
        uint4 BoneIndex   : ATTRIB4;
        float4 BoneWeight : ATTRIB5;

        #ifdef INSTANCED
            // Instance attributes
            float4 MtrxRow0      : ATTRIB6;
            float4 MtrxRow1      : ATTRIB7;
            float4 MtrxRow2      : ATTRIB8;
            float4 MtrxRow3      : ATTRIB9;
            float4 ColorOverride : ATTRIB10;
            float4 Extra         : ATTRIB11;
        #endif
    #else
        #ifdef INSTANCED
            // Instance attributes
            float4 MtrxRow0      : ATTRIB4;
            float4 MtrxRow1      : ATTRIB5;
            float4 MtrxRow2      : ATTRIB6;
            float4 MtrxRow3      : ATTRIB7;
            float4 ColorOverride : ATTRIB8;
            float4 Extra         : ATTRIB9;
        #endif
    #endif
};

struct PSInput {
    float4 Pos                      : SV_POSITION;
    float4 WorldPos                 : POSITION1;

    float4 Normal                   : NORMAL;
    float4 Tangent                  : TANGENT;

    float2 UV                       : TEX_COORD;
    float4 Color                    : COLOR;

    nointerpolation uint   TexIndex : TEX_ARRAY_INDEX;
};


void main(in VSInput VSIn, out PSInput PSIn) {
    #ifdef SKINNED
        float4 pos = boneTransform(VSIn.BoneIndex, VSIn.BoneWeight, VSIn.Pos);
    #else
        float4 pos = VSIn.Pos;
    #endif

    #ifdef INSTANCED
        float4x4 InstanceMatr = MatrixFromRows(VSIn.MtrxRow0, VSIn.MtrxRow1, VSIn.MtrxRow2, VSIn.MtrxRow3);
        TransformedData transform = applyPosTransforms(mul(pos, InstanceMatr), VSIn.UV.xy);
    #else
        TransformedData transform = applyPosTransforms(pos, VSIn.UV.xy);
    #endif

	float4 normal = VSIn.Normal * 2.0 - 1.0;
	float4 tangent = VSIn.Tangent * 2.0 - 1.0;

    PSIn.Normal   = normalize(mul(normal, Camera.world));
    PSIn.Tangent  = normalize(mul(tangent, Camera.world));

    PSIn.Pos      = transform.final;
    PSIn.WorldPos = mul(transform.pos, Camera.world);
    PSIn.UV       = VSIn.UV.xy; //applyUVTransform(VSIn.UV.xy);

    #ifdef INSTANCED
        PSIn.Color    = VSIn.ColorOverride * Constants.colorOverride;
        PSIn.TexIndex = VSIn.Extra.x;
    #else
        PSIn.Color    = Constants.colorOverride;
        PSIn.TexIndex = VSIn.UV.z;
    #endif
}
