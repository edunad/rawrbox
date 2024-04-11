#include "camera.fxh"
#include "vertex_bindless_uniforms.fxh"
#include "unpack.fxh"

#define TRANSFORM_DISPLACEMENT
#define TRANSFORM_PSX
#define TRANSFORM_BILLBOARD
#ifdef SKINNED
	#define TRANSFORM_BONES
#endif

Texture2DArray g_Textures[];
SamplerState g_Sampler;

#include "model_transforms.fxh"

struct VSInput {
	float4 Pos : ATTRIB0;
	float4 UV : ATTRIB1;

	float4 Normal : ATTRIB2;
	float4 Tangent : ATTRIB3;

#ifdef SKINNED
	uint4 BoneIndex : ATTRIB4;
	float4 BoneWeight : ATTRIB5;

	#ifdef INSTANCED
	// Instance attributes
	float4 MtrxRow0 : ATTRIB6;
	float4 MtrxRow1 : ATTRIB7;
	float4 MtrxRow2 : ATTRIB8;
	float4 MtrxRow3 : ATTRIB9;

	uint4  InstData : ATTRIB10; // Color, slice, gpu id, ??
	#endif
#else
	#ifdef INSTANCED
	// Instance attributes
	float4 MtrxRow0 : ATTRIB4;
	float4 MtrxRow1 : ATTRIB5;
	float4 MtrxRow2 : ATTRIB6;
	float4 MtrxRow3 : ATTRIB7;

	uint4  InstData : ATTRIB8; // Color, slice, gpu id, ??
	#endif
#endif
};

struct PSInput {
	float4 Pos : SV_POSITION;
	float4 WorldPos : POSITION1;

	float4 Normal : NORMAL;
	float4 Tangent : TANGENT;

	float2 UV : TEX_COORD;
	float4 Color : COLOR0;

	nointerpolation float4 GPUId : COLOR1;
	nointerpolation uint TexIndex : TEX_ARRAY_INDEX;
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

	PSIn.Normal = normalize(mul(normal, Camera.world));
	PSIn.Tangent = normalize(mul(tangent, Camera.world));

	PSIn.Pos = transform.final;
	PSIn.WorldPos = mul(transform.pos, Camera.world);
	PSIn.UV = VSIn.UV.xy;

#ifdef INSTANCED
	PSIn.Color = Unpack_RGBA8_UNORM(VSIn.InstData.x) * Unpack_RGBA8_UNORM(ColorOverride);
	PSIn.TexIndex = VSIn.UV.z + VSIn.InstData.y + SliceOverride;
	PSIn.GPUId = Unpack_ABGR8_UNORM(VSIn.InstData.z);
#else
	PSIn.Color = Unpack_RGBA8_UNORM(ColorOverride);
	PSIn.GPUId = Unpack_ABGR8_UNORM(GPUID);
	PSIn.TexIndex = VSIn.UV.z + SliceOverride;
#endif
}
