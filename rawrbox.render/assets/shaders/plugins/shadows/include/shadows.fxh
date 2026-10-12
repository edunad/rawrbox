// Based on https://ktstephano.github.io/rendering/stratusgfx/svsm

#ifndef INCLUDED_SHADOWS
#define INCLUDED_SHADOWS

#include "shadow_uniforms.fxh"

StructuredBuffer<uint> ShadowPageTable;
Texture2DArray<uint> ShadowPool;

#define SHADOW_CONTACT_HARDEN 0.25

float2 GetShadowDitherOffset(float dither) {
	float rank = round(dither * 17.0) - 1.0;
	float angle = rank * 2.39996323;

	return float2(cos(angle), sin(angle)) * sqrt((rank + 0.5) / 16.0) * 0.5;
}

uint GetShadowMip(float lod, float dither) {
	uint mip = (uint)lod;
	if (!IsShadowMipBlending(lod)) return mip;

	float threshold = (float(((uint)round(dither * 17.0) * 7u + 3u) & 15u) + 0.5) / 16.0;
	return min(mip + ((frac(lod) - (1.0 - SHADOW_MIP_BLEND)) / SHADOW_MIP_BLEND > threshold ? 1u : 0u), SHADOW_MIPS - 1u);
}

bool LoadShadowOccluder(uint index, uint2 local, out uint occluder) {
	occluder = SHADOW_EMPTY;

	uint entry = ShadowPageTable[index];
	if ((entry & (SHADOW_ALLOCATED | SHADOW_RENDERED)) != (SHADOW_ALLOCATED | SHADOW_RENDERED)) return false;

	int2 phys = (int2)GetShadowPhysTexel(entry & SHADOW_PHYS_MASK, local);
	occluder = min(ShadowPool.Load(int4(phys, SHADOW_LAYER_STATIC, 0)), ShadowPool.Load(int4(phys, SHADOW_LAYER_DYNAMIC, 0)));
	return true;
}

float GetShadowLit(uint occluder, float receiver) {
	return occluder == SHADOW_EMPTY || receiver <= asfloat(occluder) ? 1.0 : 0.0;
}

float GetShadowDither(uint centerOccluder, uint tapOccluder, float receiver, float2 offset, float depthToTexels) {
	float centerLit = GetShadowLit(centerOccluder, receiver);
	float tapLit = GetShadowLit(tapOccluder, receiver);
	if (centerLit == tapLit) return centerLit;

	float blocker = (receiver - asfloat(centerLit < 0.5 ? centerOccluder : tapOccluder)) * depthToTexels;
	return length(offset) <= blocker * SHADOW_CONTACT_HARDEN ? tapLit : centerLit;
}
// -----------

float GetClipmapShadow(uint slot, float3 worldPos, float3 normal, float dither) {
	float NdotL = dot(normal, -ShadowSlots[slot].forward.xyz);

	float slope = min(sqrt(saturate(1.0 - NdotL * NdotL)) / max(NdotL, 1e-3), SHADOW_MAX_SLOPE);
	float offsetScale = (SHADOW_NORMAL_BIAS + SHADOW_DITHER_SIZE * 0.5) * (1.0 - saturate(NdotL));

	uint level = min(GetShadowMip(GetShadowLod(slot, GetViewDepth(worldPos), GetClipmapExtent(slot), NdotL), dither), SHADOW_CLIPMAP_LEVELS - 1u);

	[loop]
	for (; level < SHADOW_CLIPMAP_LEVELS; level++) {
		float texelSize = GetClipmapTexelSize(slot, level);
		float3 biased = worldPos + normal * texelSize * offsetScale;

		float2 coord = GetClipmapPos(slot, biased) / texelSize;
		int2 center = int2(floor(coord));
		int2 origin = GetClipmapOrigin(slot, level);
		if (!IsClipmapInside(center - origin)) continue;

		uint centerOccluder;
		if (!LoadShadowOccluder(GetShadowEntry(slot, level, 0u, GetClipmapTablePage(center)), uint2(center & int(SHADOW_PAGE_SIZE - 1u)), centerOccluder)) continue;

		float receiver = GetClipmapDepth(slot, biased) - texelSize * (1.0 + (0.5 + SHADOW_DITHER_SIZE * 0.5) * slope);
		if (SHADOW_DITHER_SIZE <= 0.0) return GetShadowLit(centerOccluder, receiver);

		float2 offset = GetShadowDitherOffset(dither) * SHADOW_DITHER_SIZE;
		int2 tap = int2(floor(coord + offset));

		uint tapOccluder;
		if (!IsClipmapInside(tap - origin) || !LoadShadowOccluder(GetShadowEntry(slot, level, 0u, GetClipmapTablePage(tap)), uint2(tap & int(SHADOW_PAGE_SIZE - 1u)), tapOccluder)) tapOccluder = centerOccluder;

		return GetShadowDither(centerOccluder, tapOccluder, receiver, offset, 1.0 / texelSize);
	}

	return 1.0;
}

float GetShadow(Light light, float3 worldPos, float3 normal, float dither) {
	uint shadow = GetLightShadow(light);
	if (shadow == 0u) return 1.0;

	uint slot = shadow - 1u;
	if (IsShadowClipmap(slot)) return GetClipmapShadow(slot, worldPos, normal, dither);

	float3 dir = worldPos - light.position;
	float axisDistance = GetShadowAxisDistance(dir);

	float NdotL = dot(normal, -normalize(dir));
	float slope = min(sqrt(saturate(1.0 - NdotL * NdotL)) / max(NdotL, 1e-3), SHADOW_MAX_SLOPE);
	float offsetScale = (SHADOW_NORMAL_BIAS + SHADOW_DITHER_SIZE * 0.5) * (1.0 - saturate(NdotL));

	[loop]
	for (uint mip = GetShadowMip(GetShadowLod(slot, GetViewDepth(worldPos), axisDistance, NdotL), dither); mip < SHADOW_MIPS; mip++) {
		float texelSize = GetShadowTexelSize(axisDistance, mip);
		float3 biasDir = dir + normal * texelSize * offsetScale;

		uint face = GetShadowFace(biasDir);
		float size = SHADOW_VIRTUAL_SIZE >> mip;
		float2 coord = saturate(GetShadowFaceUV(biasDir, face)) * size;
		uint2 center = (uint2)clamp(coord, 0.0, size - 1.0);

		uint centerOccluder;
		if (!LoadShadowOccluder(GetShadowEntry(slot, face, mip, center / SHADOW_PAGE_SIZE), center % SHADOW_PAGE_SIZE, centerOccluder)) continue;

		float slopeBias = texelSize * (0.5 + SHADOW_DITHER_SIZE * 0.5) * slope / light.radius;
		float receiver = length(biasDir) / light.radius - SHADOW_DEPTH_BIAS - slopeBias;
		if (SHADOW_DITHER_SIZE <= 0.0) return GetShadowLit(centerOccluder, receiver);

		float2 offset = GetShadowDitherOffset(dither) * SHADOW_DITHER_SIZE;
		uint2 tap = (uint2)clamp(coord + offset, 0.0, size - 1.0);

		uint tapOccluder;
		if (!LoadShadowOccluder(GetShadowEntry(slot, face, mip, tap / SHADOW_PAGE_SIZE), tap % SHADOW_PAGE_SIZE, tapOccluder)) tapOccluder = centerOccluder;

		return GetShadowDither(centerOccluder, tapOccluder, receiver, offset, light.radius / texelSize);
	}

	return 1.0;
}

#endif