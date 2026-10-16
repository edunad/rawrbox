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

	float2 Normal : ATTRIB2; // PACKED
	uint2 Tangent : ATTRIB3; // PACKED

#ifdef SKINNED
	uint4 BoneIndex : ATTRIB4;
	float4 BoneWeight : ATTRIB5;

	#ifdef INSTANCED
	uint InstanceIndex : ATTRIB6;
	#endif
#else
	#ifdef INSTANCED
	uint InstanceIndex : ATTRIB4;
	#endif
#endif
};

struct PSInput {
	float4 Pos : SV_POSITION;
	float3 WorldPos : POSITION1;

	float3 Normal : NORMAL;
	float4 Tangent : TANGENT; // xyz + sign

	float2 UV : TEX_COORD;

	nointerpolation uint3 Data : DATA; // Color (RGBA8) + GPU id (ABGR8) +  texture slice
};

void main(in VSInput VSIn, out PSInput PSIn) {
	float4 pos = float4(VSIn.Pos.xyz, 1.0);

	float3 normal = UnpackOctNormal(VSIn.Normal);
	float4 tangent = UnpackOctTangent(VSIn.Tangent);

#ifdef SKINNED
	float4x4 skin = boneMatrix(VSIn.BoneIndex, VSIn.BoneWeight);

	pos = mul(skin, pos);
	normal = mul((float3x3)skin, normal);
	tangent.xyz = mul((float3x3)skin, tangent.xyz);
#endif

#ifdef INSTANCED
	Instance instance = Instances[VSIn.InstanceIndex];
	float3x4 instanceMatr = GetInstanceMatrix(instance);

	pos = float4(mul(instanceMatr, pos), pos.w);
	normal = mul((float3x3)instanceMatr, normal);
	tangent.xyz = mul((float3x3)instanceMatr, tangent.xyz);
#endif

	TransformedData transform = applyPosTransforms(pos, VSIn.UV);

	float3x3 world = (float3x3)Constants.world;
	PSIn.Normal = normalize(mul(normal, world));
	PSIn.Tangent = float4(normalize(mul(tangent.xyz, world)), tangent.w);

	PSIn.Pos = transform.final;
	PSIn.WorldPos = mul(transform.pos, Constants.world).xyz;
	PSIn.UV = VSIn.UV;

#ifdef INSTANCED
	PSIn.Data = uint3(
	    Pack_RGBA8_UNORM(Unpack_RGBA8_UNORM(instance.data.x) * Unpack_RGBA8_UNORM(ColorOverride)),
	    instance.data.z,
	    (uint)VSIn.Pos.w + instance.data.y + SliceOverride);
#else
	PSIn.Data = uint3(ColorOverride, GPUID, (uint)VSIn.Pos.w + SliceOverride);
#endif
}
