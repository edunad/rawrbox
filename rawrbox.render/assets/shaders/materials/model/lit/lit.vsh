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
	float4 Pos : ATTRIB0; // xyz = position, w = texture
	float2 UV : ATTRIB1;

	float2 Normal : ATTRIB2; // PACKED
	uint2 Tangent : ATTRIB3; // PACKED

#ifdef SKINNED
	uint4 BoneIndex : ATTRIB4;
	float4 BoneWeight : ATTRIB5;

	#ifdef INSTANCED
	// Instance attributes
	float4 MtrxRow0 : ATTRIB6;
	float4 MtrxRow1 : ATTRIB7;
	float4 MtrxRow2 : ATTRIB8;
	float4 MtrxRow3 : ATTRIB9;

	uint4 InstData : ATTRIB10; // Color, slice, gpu id, ??
	#endif
#else
	#ifdef INSTANCED
	// Instance attributes
	float4 MtrxRow0 : ATTRIB4;
	float4 MtrxRow1 : ATTRIB5;
	float4 MtrxRow2 : ATTRIB6;
	float4 MtrxRow3 : ATTRIB7;

	uint4 InstData : ATTRIB8; // Color, slice, gpu id, ??
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
	float4x4 InstanceMatr = MatrixFromRows(VSIn.MtrxRow0, VSIn.MtrxRow1, VSIn.MtrxRow2, VSIn.MtrxRow3);

	pos = mul(pos, InstanceMatr);
	normal = mul(normal, (float3x3)InstanceMatr);
	tangent.xyz = mul(tangent.xyz, (float3x3)InstanceMatr);
#endif

	TransformedData transform = applyPosTransforms(pos, VSIn.UV);

	float3x3 world = (float3x3)Camera.world;
	PSIn.Normal = normalize(mul(normal, world));
	PSIn.Tangent = float4(normalize(mul(tangent.xyz, world)), tangent.w);

	PSIn.Pos = transform.final;
	PSIn.WorldPos = mul(transform.pos, Camera.world).xyz;
	PSIn.UV = VSIn.UV;

#ifdef INSTANCED
	PSIn.Data = uint3(
	    Pack_RGBA8_UNORM(Unpack_RGBA8_UNORM(VSIn.InstData.x) * Unpack_RGBA8_UNORM(ColorOverride)),
	    VSIn.InstData.z,
	    (uint)VSIn.Pos.w + VSIn.InstData.y + SliceOverride);
#else
	PSIn.Data = uint3(ColorOverride, GPUID, (uint)VSIn.Pos.w + SliceOverride);
#endif
}
