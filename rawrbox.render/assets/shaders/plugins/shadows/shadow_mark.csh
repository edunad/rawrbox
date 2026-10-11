#include "camera.fxh"
#include "light_uniforms.fxh"
#include "math.fxh"
#include "shadow_uniforms.fxh"

#define READ_LIGHTS
#define READ_TILES
#include "binning.fxh"
#include "lighting.fxh"

Texture2DArray<float> CameraDepth;
RWStructuredBuffer<uint> ShadowRequests;
RWStructuredBuffer<uint> ShadowReceiverDepth;

#define SHADOW_MARK_BACKFACE -0.25

void MarkEntry(uint index) {
	if (ShadowRequests[index] == SHADOW_FRAME) return;
	ShadowRequests[index] = SHADOW_FRAME;
}

uint GetPageEntry(uint slot, float3 dir, uint mip) {
	uint face = GetShadowFace(dir);
	return GetShadowEntry(slot, face, mip, GetShadowTexel(dir, face, mip) / SHADOW_PAGE_SIZE);
}

void MarkPages(Light light, uint slot, float3 dir, float axisDistance, uint mip) {
	uint face = GetShadowFace(dir);
	uint2 texel = GetShadowTexel(dir, face, mip);
	uint2 local = texel % SHADOW_PAGE_SIZE;

	uint margin = (uint)ceil(2.0 * (SHADOW_NORMAL_BIAS + SHADOW_DITHER_SIZE)) + 1u;
	if (all(local >= margin) && all(local + margin < SHADOW_PAGE_SIZE)) {
		MarkEntry(GetShadowEntry(slot, face, mip, texel / SHADOW_PAGE_SIZE));
		return;
	}

	float bias = GetShadowTexelSize(axisDistance, mip) * (SHADOW_NORMAL_BIAS + SHADOW_DITHER_SIZE);
	bool spot = GetLightType(light) == LIGHT_SPOT;
	uint last = 0xFFFFFFFFu;

	for (uint corner = 0; corner < 8; corner++) {
		float3 offset = float3((corner & 1u) != 0 ? bias : -bias, (corner & 2u) != 0 ? bias : -bias, (corner & 4u) != 0 ? bias : -bias);
		if (spot && dot(normalize(dir + offset), light.direction) < light.cosUmbra) continue;

		uint index = GetPageEntry(slot, dir + offset, mip);
		if (index == last) continue;

		MarkEntry(index);
		last = index;
	}
}

void MarkLight(Light light, float3 worldPos, float3 normal, float viewDepth) {
	uint shadow = GetLightShadow(light);
	if (shadow == 0u) return;

	float3 L;
	if (GetAttenuation(light, worldPos, L) <= 0.0) return;

	uint slot = shadow - 1u;
	float3 dir = worldPos - light.position;
	float axisDistance = GetShadowAxisDistance(dir);

	float NdotL = dot(normal, -normalize(dir));
	if (NdotL < SHADOW_MARK_BACKFACE) return;

	uint receiver = slot * 6u + GetShadowFace(dir);
	uint depth = asuint(length(dir) / light.radius);
	if (depth > ShadowReceiverDepth[receiver]) InterlockedMax(ShadowReceiverDepth[receiver], depth);

	float lod = GetShadowLod(slot, viewDepth, axisDistance, NdotL);
	uint mip = (uint)lod;

	MarkPages(light, slot, dir, axisDistance, mip);
	if (mip + 1u < SHADOW_MIPS) MarkPages(light, slot, dir, axisDistance, mip + 1u);
}

// DIRECTIONAL LIGHTS ----
void MarkClipmapPages(uint slot, uint level, int2 local) {
	int margin = (int)ceil(SHADOW_NORMAL_BIAS + SHADOW_DITHER_SIZE) + 1;

	int2 low = clamp(local - margin, 0, int(SHADOW_VIRTUAL_SIZE) - 1);
	int2 high = clamp(local + margin, 0, int(SHADOW_VIRTUAL_SIZE) - 1);

	uint2 first = uint2(low) / SHADOW_PAGE_SIZE;
	uint2 last = uint2(high) / SHADOW_PAGE_SIZE;

	for (uint y = first.y; y <= last.y; y++) {
		for (uint x = first.x; x <= last.x; x++) {
			MarkEntry(GetShadowEntry(slot, level, 0u, GetShadowTablePage(slot, level, uint2(x, y))));
		}
	}
}

void MarkClipmap(Light light, float3 worldPos, float3 normal, float viewDepth) {
	uint shadow = GetLightShadow(light);
	if (shadow == 0u) return;

	uint slot = shadow - 1u;
	float NdotL = dot(normal, -ShadowSlots[slot].forward.xyz);
	if (NdotL < SHADOW_MARK_BACKFACE) return;

	float lod = GetShadowLod(slot, viewDepth, GetClipmapExtent(slot), NdotL);
	uint marked = 0u;
	for (uint level = min((uint)lod, SHADOW_CLIPMAP_LEVELS - 1u); level < SHADOW_CLIPMAP_LEVELS && marked < 2u; level++) {
		int2 local = GetClipmapTexel(slot, level, worldPos) - GetClipmapOrigin(slot, level);
		if (!IsClipmapInside(local)) continue;

		MarkClipmapPages(slot, level, local);
		marked++;
	}
}
// -----------

float3 GetViewPosition(int2 pixel) {
	int2 clamped = clamp(pixel, int2(0, 0), int2(ScreenSize) - 1);
	float depth = CameraDepth.Load(int4(clamped, 0, 0));

	float2 ndc = float2((clamped.x + 0.5) / ScreenSize.x * 2.0 - 1.0, 1.0 - (clamped.y + 0.5) / ScreenSize.y * 2.0);
	float viewDepth = (2.0 * NearFar.x * NearFar.y) / (NearFar.y + NearFar.x - depth * (NearFar.y - NearFar.x));

	return float3(ndc.x * viewDepth / SCamera.proj[0][0], ndc.y * viewDepth / SCamera.proj[1][1], viewDepth);
}

float3 GetViewNormal(int2 pixel, float3 center) {
	float3 left = GetViewPosition(pixel - int2(1, 0)) - center;
	float3 right = GetViewPosition(pixel + int2(1, 0)) - center;
	float3 up = GetViewPosition(pixel - int2(0, 1)) - center;
	float3 down = GetViewPosition(pixel + int2(0, 1)) - center;

	float3 dx = abs(right.z) < abs(left.z) ? right : -left;
	float3 dy = abs(down.z) < abs(up.z) ? down : -up;
	float3 normal = normalize(cross(dx, dy));

	return dot(normal, center) > 0.0 ? -normal : normal;
}

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID) {
	if (id.x >= (uint)ScreenSize.x || id.y >= (uint)ScreenSize.y) return;
	if (TOTAL_LIGHTS == 0) return;

	float depth = CameraDepth.Load(int4(id.xy, 0, 0));
	if (depth >= 1.0) return;

	// REBUILD --
	float2 pixel = float2(id.xy) + 0.5;
	float3 viewPos = GetViewPosition(int2(id.xy));
	float viewDepth = viewPos.z;

	float3 worldPos = mul(float4(viewPos, 1.0), ShadowConstants.invView).xyz;
	float3 normal = normalize(mul(float4(GetViewNormal(int2(id.xy), viewPos), 0.0), ShadowConstants.invView).xyz);
	// --------------

	for (uint directional = 0; directional < TOTAL_DIRECTIONAL_LIGHTS; directional++) {
		MarkClipmap(GetLight(directional), worldPos, normal, viewDepth);
	}

	uint tileIndex = GetTileIndex(pixel);
	uint2 range = UnpackZBin(LightZBins[GetZBin(viewDepth)]);

	for (uint bin = range.x / BIN_BITS; bin <= range.y / BIN_BITS; ++bin) {
		uint mask = LightTiles[tileIndex * LIGHT_BINS + bin] & GetBinMask(bin, range);

		while (mask) {
			uint bitIndex = firstbitlow(mask);
			mask ^= 1u << bitIndex;

			uint index = bitIndex + bin * BIN_BITS;
			if (index >= TOTAL_LIGHTS) break;

			MarkLight(GetLight(index), worldPos, normal, viewDepth);
		}
	}
}