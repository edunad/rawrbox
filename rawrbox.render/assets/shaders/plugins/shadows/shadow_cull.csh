#include "shadow_uniforms.fxh"
#include "instance.fxh"

#define CULL_PARAMS 5u

struct ShadowDrawStruct {
	uint4 data;
};

ConstantBuffer<ShadowDrawStruct> ShadowDraw;

RWStructuredBuffer<uint4> ShadowCull;
RWStructuredBuffer<uint> ShadowIndices;

float GetMaxScale(float3x4 mtx) {
	float3 x = float3(mtx[0][0], mtx[1][0], mtx[2][0]);
	float3 y = float3(mtx[0][1], mtx[1][1], mtx[2][1]);
	float3 z = float3(mtx[0][2], mtx[1][2], mtx[2][2]);

	return sqrt(max(dot(x, x), max(dot(y, y), dot(z, z))));
}

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID) {
	uint entry = ShadowDraw.data.x;
	uint4 settings = ShadowCull[entry + 4u]; // Instances, slots, list capacity
	if (id.x >= settings.x) return;

	// WORLD SPHERE ---
	float3x4 model = float3x4(asfloat(ShadowCull[entry]), asfloat(ShadowCull[entry + 1u]), asfloat(ShadowCull[entry + 2u]));
	float3x4 instance = GetInstanceMatrix(Instances[id.x]);
	float4 bounds = asfloat(ShadowCull[entry + 3u]);

	float3 center = mul(model, float4(mul(instance, float4(bounds.xyz, 1.0)), 1.0));
	float radius = bounds.w * GetMaxScale(model) * GetMaxScale(instance);
	// ------

	uint counter = entry + CULL_PARAMS;
	bool scatter = ShadowDraw.data.y != 0u;

	uint first = 0u;
	for (uint slot = 0u; slot < settings.y; slot++) {
		float4 light = ShadowSlots[slot].light;
		float3 offset = center - light.xyz;
		float reach = light.w + radius;

		bool hit = !IsShadowClipmap(slot) && dot(offset, offset) <= reach * reach;

		if (!scatter) {
			if (hit) InterlockedAdd(ShadowCull[counter + slot].x, 1u);
			continue;
		}

		// SCATTER ---
		uint count = ShadowCull[counter + slot].x;

		if (hit) {
			uint index;
			InterlockedAdd(ShadowCull[counter + slot].y, 1u, index);
			if (first + index < settings.z) ShadowIndices[first + index] = id.x;
		}

		if (id.x == 0u) ShadowCull[counter + slot].zw = uint2(min(first, settings.z), first < settings.z ? min(count, settings.z - first) : 0u);
		first += count;
		// ------
	}
}