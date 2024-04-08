#include "camera.fxh"
#include "model_transforms.fxh"

#include "particles_uniforms.fxh"

struct GSInput {
    float4 POS         : SV_POSITION;
    float2 SIZE        : SIZE;
    float3x3 ROTATION  : ROTATION;
    float3 UV          : TEXCOORD0;
    float4 COLOR       : COLOR;
};

struct GSOutput {
    float4 Position  : SV_POSITION;
    float3 UV        : TEXCOORD0;
    float4 Color     : COLOR;
    uint   TextureID : TEX_ARRAY_INDEX;
};



[maxvertexcount(4)]
void main(point GSInput input[1], inout TriangleStream<GSOutput> outputStream) {
    float2 halfSize = input[0].SIZE * 0.5f;

    // Define offsets for quad vertices
    float3 offsets[4] = {
        float3(-halfSize.x, halfSize.y, 0.0f),  // Top Left
        float3(halfSize.x, halfSize.y, 0.0f),   // Top Right
        float3(-halfSize.x, -halfSize.y, 0.0f), // Bottom Left
        float3(halfSize.x, -halfSize.y, 0.0f)   // Bottom Right
    };

    // UV coordinates for each vertex of the quad
    float2 uvs[4] = {
        float2(0.0f, 0.0f), // Top Left
        float2(1.0f, 0.0f), // Top Right
        float2(0.0f, 1.0f), // Bottom Left
        float2(1.0f, 1.0f)  // Bottom Right
    };

    GSOutput output;
    output.Color = input[0].COLOR;
    output.TextureID = EmitterConstants.textureID;

    [unroll]
    for (int i = 0; i < 4; ++i) {
        float3 rotatedOffset = mul(input[0].ROTATION, offsets[i]);
        float4 billboardOffset = billboardTransform(float4(rotatedOffset, 1.0), EmitterConstants.billboard);

        output.Position = mul(float4(input[0].POS.xyz + billboardOffset.xyz, 1.0f), Camera.worldViewProj);
        output.UV = float3(uvs[i], input[0].UV.z);

        outputStream.Append(output);
    }

    outputStream.RestartStrip();
}
