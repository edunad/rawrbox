#include "include/camera.fxh"
#include "include/unpack.fxh"
#include "include/bindless/vertex_bindless_uniforms.fxh"

#define TRANSFORM_DISPLACEMENT
#ifdef SKINNED
	#define TRANSFORM_BONES
#endif

StructuredBuffer<uint> ShadowViews;

#include "plugins/shadows/include/shadow_uniforms.fxh"

#ifdef INSTANCED
	#include "include/instance.fxh"
#endif

Texture2DArray g_Textures[];
SamplerState g_Sampler;

#include "materials/include/model_transforms.fxh"

struct ShadowDrawStruct {
	uint4 data;
};

ConstantBuffer<ShadowDrawStruct> ShadowDraw;

struct VSInput {
	float4 Pos : ATTRIB0; // xyz = position, w = texture
	float2 UV : ATTRIB1;

#ifdef SHADOW_NORMALS
	#ifdef SKINNED
	    uint4 BoneIndex : ATTRIB4;
	    float4 BoneWeight : ATTRIB5;

        #ifdef INSTANCED
            uint InstanceIndex : ATTRIB6;
        #endif
	#elif defined(INSTANCED)
	    uint InstanceIndex : ATTRIB4;
	#endif
#else
	#ifdef SKINNED
	    uint4 BoneIndex : ATTRIB2;
	    float4 BoneWeight : ATTRIB3;

		#ifdef INSTANCED
	        uint InstanceIndex : ATTRIB4;
		#endif
	#elif defined(INSTANCED)
	    uint InstanceIndex : ATTRIB2;
	#endif
#endif
};

struct GSInput {
	float4 Pos : SV_POSITION;
	float3 Dir : POSITION1;
	float2 UV : TEX_COORD;

	nointerpolation uint View : VIEW;
};

void main(in VSInput VSIn, in uint instance : SV_InstanceID, out GSInput GSIn) {
#ifdef INSTANCED
	uint local = ShadowDraw.data.y;
#else
	uint local = ShadowDraw.data.y + instance;
#endif

	uint view = ShadowViews[ShadowDraw.data.x + local];
	uint face = GetShadowViewFace(view);
	float4 light = ShadowSlots[GetShadowViewSlot(view)].light;

	float4 pos = float4(VSIn.Pos.xyz, 1.0);
#ifdef SKINNED
	pos = mul(boneMatrix(VSIn.BoneIndex, VSIn.BoneWeight), pos);
#endif

#ifdef INSTANCED
	Instance instanceData = Instances[VSIn.InstanceIndex];
	pos = float4(mul(GetInstanceMatrix(instanceData), pos), pos.w);
#endif

	TransformedData transform = applyPosTransforms(pos, VSIn.UV);
	float3 world = mul(transform.pos, Constants.world).xyz;
	uint slot = GetShadowViewSlot(view);

	if (IsShadowClipmap(slot)) {
		float2 local = GetClipmapPos(slot, world) / GetClipmapTexelSize(slot, face) - float2(GetClipmapOrigin(slot, face));

		GSIn.Pos = float4(local.x / SHADOW_VIRTUAL_SIZE * 2.0 - 1.0, 1.0 - local.y / SHADOW_VIRTUAL_SIZE * 2.0, 0.5, 1.0);
		GSIn.Dir = float3(GetClipmapDepth(slot, world), 0.0, 0.0);
	} else {
		float3 dir = world - light.xyz;
		float3 clip = GetShadowFaceClip(dir, face);

		GSIn.Pos = float4(clip.x, clip.y, clip.z - SHADOW_NEAR, clip.z);
		GSIn.Dir = dir / light.w;
	}

	GSIn.UV = VSIn.UV;

#ifdef INSTANCED
	GSIn.View = view | (((uint)VSIn.Pos.w + instanceData.data.y + SliceOverride) << SHADOW_SLICE_SHIFT);
#else
	GSIn.View = view | (((uint)VSIn.Pos.w + SliceOverride) << SHADOW_SLICE_SHIFT);
#endif
}