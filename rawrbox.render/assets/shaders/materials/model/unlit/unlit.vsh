#include "include/camera.fxh"
#include "include/unpack.fxh"
#include "include/bindless/vertex_bindless_uniforms.fxh"

#define TRANSFORM_DISPLACEMENT
#define TRANSFORM_PSX
#define TRANSFORM_BILLBOARD
#ifdef SKINNED
	#define TRANSFORM_BONES
#endif

#ifdef INSTANCED
	#include "include/instance.fxh"
#endif

Texture2DArray g_Textures[];
SamplerState g_Sampler;

#include "materials/include/model_transforms.fxh"

struct VSInput {
	float4 Pos : ATTRIB0; // xyz = position, w = texture
	float2 UV : ATTRIB1;

#ifdef SKINNED
	uint4 BoneIndex : ATTRIB2;
	float4 BoneWeight : ATTRIB3;

	#ifdef INSTANCED
	uint InstanceIndex : ATTRIB4;
	#endif
#else
	#ifdef INSTANCED
	uint InstanceIndex : ATTRIB2;
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
	float4 pos = boneTransform(VSIn.BoneIndex, VSIn.BoneWeight, float4(VSIn.Pos.xyz, 1.0));
#else
	float4 pos = float4(VSIn.Pos.xyz, 1.0);
#endif

#ifdef INSTANCED
	Instance instance = Instances[VSIn.InstanceIndex];
	TransformedData transform = applyPosTransforms(float4(mul(GetInstanceMatrix(instance), pos), pos.w), VSIn.UV);
#else
	TransformedData transform = applyPosTransforms(pos, VSIn.UV);
#endif

	PSIn.Pos = transform.final;
	PSIn.WorldPos = mul(transform.pos, Constants.world);
	PSIn.UV = VSIn.UV;

#ifdef INSTANCED
	PSIn.Color = Unpack_RGBA8_UNORM(instance.data.x) * Unpack_RGBA8_UNORM(ColorOverride);
	PSIn.TexIndex = VSIn.Pos.w + instance.data.y + SliceOverride;
	PSIn.GPUId = Unpack_ABGR8_UNORM(instance.data.z);
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
	PSIn.TexIndex = VSIn.Pos.w + SliceOverride;
#endif
}
