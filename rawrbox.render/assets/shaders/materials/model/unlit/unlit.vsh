#include "camera.fxh"
#include "unpack.fxh"
#include "vertex_bindless_uniforms.fxh"

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

#ifdef SKINNED
	uint4 BoneIndex : ATTRIB2;
	float4 BoneWeight : ATTRIB3;

	#ifdef INSTANCED
	// Instance attributes
	float4 MtrxRow0 : ATTRIB4;
	float4 MtrxRow1 : ATTRIB5;
	float4 MtrxRow2 : ATTRIB6;
	float4 MtrxRow3 : ATTRIB7;

	uint4 InstData : ATTRIB8; // Color, slice, gpu id, ??
	#endif
#else
	#ifdef INSTANCED
	// Instance attributes
	float4 MtrxRow0 : ATTRIB2;
	float4 MtrxRow1 : ATTRIB3;
	float4 MtrxRow2 : ATTRIB4;
	float4 MtrxRow3 : ATTRIB5;

	uint4 InstData : ATTRIB6; // Color, slice, gpu id, ??
	#endif
#endif
};

struct PSInput {
	float4 Pos : SV_POSITION;
	float4 WorldPos : POSITION1;

	float2 UV : TEX_COORD0;
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

	PSIn.Pos = transform.final;
	PSIn.WorldPos = mul(transform.pos, Camera.world);
	PSIn.UV = VSIn.UV.xy;

#ifdef INSTANCED
	PSIn.Color = Unpack_RGBA8_UNORM(VSIn.InstData.x) * Unpack_RGBA8_UNORM(ColorOverride);
	PSIn.TexIndex = VSIn.UV.z + VSIn.InstData.y + SliceOverride;
	PSIn.GPUId = Unpack_ABGR8_UNORM(VSIn.InstData.z);
#else

// DEBUG ----
#ifdef SHADER_DEBUG
	#ifdef SKINNED
		PSIn.Color = boneToColor(VSIn.BoneIndex, VSIn.BoneWeight, 4);
	#endif
#else
	PSIn.Color = Unpack_RGBA8_UNORM(ColorOverride);
#endif
// --------------

	PSIn.GPUId = Unpack_ABGR8_UNORM(GPUID);
	PSIn.TexIndex = VSIn.UV.z + SliceOverride;
#endif
}
