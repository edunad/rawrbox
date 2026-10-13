#ifndef INCLUDED_SHADOW_UNIFORMS
#define INCLUDED_SHADOW_UNIFORMS

struct ShadowConstantsStruct {
	float4x4 invView;

	uint4 settings;
	float4 params;
	uint4 misc;
};

struct ShadowSlotStruct {
	float4 right;
	float4 up;
	float4 forward;

	int4 origins[3];
	float4 params;
	float4 light;
};

ConstantBuffer<ShadowConstantsStruct> ShadowConstants;
StructuredBuffer<ShadowSlotStruct> ShadowSlots;

#define SHADOW_FRAME            ShadowConstants.settings.x
#define SHADOW_ACTIVE_SLOTS     ShadowConstants.settings.y
#define SHADOW_INVALIDATIONS    ShadowConstants.settings.z
#define SHADOW_DITHER_SIZE      asfloat(ShadowConstants.settings.w)

#define SHADOW_LOD_BIAS         ShadowConstants.params.x
#define SHADOW_PIXEL_SCALE      ShadowConstants.params.y
#define SHADOW_NORMAL_BIAS      ShadowConstants.params.z
#define SHADOW_DEPTH_BIAS       ShadowConstants.params.w

#ifdef _DEBUG
	#define SHADOW_DEBUG_MODE  ShadowConstants.misc.x
	#define SHADOW_DEBUG_DEPTH ShadowConstants.misc.y
#endif
#define SHADOW_RENDER_BUDGET    ShadowConstants.misc.z
#define SHADOW_DRAWS            ShadowConstants.misc.w

// PAGE TABLE ----
#define SHADOW_LIGHT_ENTRIES (SHADOW_FACE_ENTRIES * 6u)
#define SHADOW_TOTAL_PAGES   (SHADOW_POOL_PAGES * SHADOW_POOL_PAGES)

#define SHADOW_PHYS_MASK     0x0000FFFFu
#define SHADOW_ALLOCATED     0x40000000u
#define SHADOW_RENDERED      0x80000000u
#define SHADOW_DRAW          0x20000000u
#define SHADOW_EMPTY         0xFFFFFFFFu
#define SHADOW_COUNTED       0x80000000u

#define SHADOW_LAYER_STATIC  0u
#define SHADOW_LAYER_DYNAMIC 1u

#define SHADOW_CLEAR_STATIC  1u
#define SHADOW_CLEAR_DYNAMIC 2u
// ---------------

// VIEWS ----
#define SHADOW_NEAR 0.001
#define SHADOW_ALL_VIEWS 0xFFFFFFFFu

#define SHADOW_MIP_BLEND 0.35
#define SHADOW_SLOPE_MIN_COS 0.25
#define SHADOW_MAX_SLOPE     4.0

// Slot 13 + face 3 + mip 4 + layer 1 ---
#define SHADOW_SLOT_BITS  13u
#define SHADOW_SLICE_SHIFT 21u
#define SHADOW_CLIPMAP_LEVELS 6u

uint GetShadowViewSlot(uint view) { return view & ((1u << SHADOW_SLOT_BITS) - 1u); }
uint GetShadowViewFace(uint view) { return (view >> SHADOW_SLOT_BITS) & 0x7u; }
uint GetShadowViewMip(uint view) { return (view >> 16u) & 0xFu; }
uint GetShadowViewLayer(uint view) { return (view >> 20u) & 0x1u; }
uint GetShadowViewSlice(uint view) { return view >> SHADOW_SLICE_SHIFT; }

uint PackShadowView(uint slot, uint face, uint mip, uint layer) {
    return slot | (face << SHADOW_SLOT_BITS) | (mip << 16u) | (layer << 20u);
}
// -----------

uint GetShadowBatch(uint slot, uint face, uint layer) { return (slot * 6u + face) * 2u + layer; }

uint4 UnpackShadowRect(uint rect) { return uint4(rect & 0xFFu, (rect >> 8u) & 0xFFu, (rect >> 16u) & 0xFFu, rect >> 24u); }
bool ShadowRectsOverlap(uint4 a, uint4 b) { return a.x <= a.z && b.x <= b.z && a.x <= b.z && b.x <= a.z && a.y <= b.w && b.y <= a.w; }
// ---------------

// CUBE FACES ----
static const float3 SHADOW_FACE_FORWARD[6] = {float3(1, 0, 0), float3(-1, 0, 0), float3(0, 1, 0), float3(0, -1, 0), float3(0, 0, 1), float3(0, 0, -1)};
static const float3 SHADOW_FACE_RIGHT[6] = {float3(0, 0, -1), float3(0, 0, 1), float3(1, 0, 0), float3(1, 0, 0), float3(1, 0, 0), float3(-1, 0, 0)};
static const float3 SHADOW_FACE_UP[6] = {float3(0, 1, 0), float3(0, 1, 0), float3(0, 0, -1), float3(0, 0, 1), float3(0, 1, 0), float3(0, 1, 0)};

uint GetShadowFace(float3 dir) {
	float3 a = abs(dir);

	if (a.x >= a.y && a.x >= a.z) return dir.x >= 0.0 ? 0u : 1u;
	if (a.y >= a.z) return dir.y >= 0.0 ? 2u : 3u;

	return dir.z >= 0.0 ? 4u : 5u;
}

float3 GetShadowFaceClip(float3 dir, uint face) {
	return float3(dot(dir, SHADOW_FACE_RIGHT[face]), dot(dir, SHADOW_FACE_UP[face]), dot(dir, SHADOW_FACE_FORWARD[face]));
}

float2 GetShadowFaceUV(float3 dir, uint face) {
	float3 clip = GetShadowFaceClip(dir, face);
	float depth = max(clip.z, 1e-6);

	return float2(clip.x / depth * 0.5 + 0.5, 0.5 - clip.y / depth * 0.5);
}

float GetShadowAxisDistance(float3 dir) {
	float3 a = abs(dir);
	return max(a.x, max(a.y, a.z));
}
// ---------------

// CLIPMAPS ----
bool IsShadowClipmap(uint slot) { return ShadowSlots[slot].up.w > 0.5; }
float GetClipmapExtent(uint slot) { return ShadowSlots[slot].right.w; }

float GetClipmapTexelSize(uint slot, uint level) {
	return 2.0 * GetClipmapExtent(slot) * float(1u << level) / SHADOW_VIRTUAL_SIZE;
}

int2 GetClipmapOrigin(uint slot, uint level) {
	int4 origins = ShadowSlots[slot].origins[level / 2u];
	return (level & 1u) != 0u ? origins.zw : origins.xy;
}

float2 GetClipmapPos(uint slot, float3 world) {
	return float2(dot(world, ShadowSlots[slot].right.xyz), dot(world, ShadowSlots[slot].up.xyz));
}

int2 GetClipmapTexel(uint slot, uint level, float3 world) {
	return int2(floor(GetClipmapPos(slot, world) / GetClipmapTexelSize(slot, level)));
}

float GetClipmapDepth(uint slot, float3 world) {
	return dot(world, ShadowSlots[slot].forward.xyz) + ShadowSlots[slot].forward.w;
}

bool IsClipmapInside(int2 local) {
	return all(local >= 0) && all(local < int(SHADOW_VIRTUAL_SIZE));
}

uint2 GetClipmapTablePage(int2 texel) {
	return uint2(texel & int(SHADOW_VIRTUAL_SIZE - 1u)) / SHADOW_PAGE_SIZE;
}

uint2 GetShadowTablePage(uint slot, uint face, uint2 page) {
	if (!IsShadowClipmap(slot)) return page;
	return uint2((int2(page) + GetClipmapOrigin(slot, face) / int(SHADOW_PAGE_SIZE)) & int(SHADOW_PAGES - 1u));
}

uint2 GetShadowWindowPage(uint slot, uint face, uint2 page) {
	if (!IsShadowClipmap(slot)) return page;
	return uint2((int2(page) - GetClipmapOrigin(slot, face) / int(SHADOW_PAGE_SIZE)) & int(SHADOW_PAGES - 1u));
}
// ---------------

// PAGES ----
uint GetShadowMipOffset(uint mip) {
	uint pages = SHADOW_PAGES >> mip;
	return (4u * (SHADOW_PAGES * SHADOW_PAGES - pages * pages)) / 3u;
}

uint GetShadowEntry(uint slot, uint face, uint mip, uint2 page) {
	return slot * SHADOW_LIGHT_ENTRIES + face * SHADOW_FACE_ENTRIES + GetShadowMipOffset(mip) + page.y * (SHADOW_PAGES >> mip) + page.x;
}

float GetShadowLod(uint slot, float viewDepth, float axisDistance, float NdotL) {
	float footprint = max(viewDepth * SHADOW_PIXEL_SCALE, 1e-6);
	float texel = max(2.0 * axisDistance / SHADOW_VIRTUAL_SIZE, 1e-6);
	float slope = NdotL >= SHADOW_SLOPE_MIN_COS ? NdotL : lerp(1.0, SHADOW_SLOPE_MIN_COS, saturate(NdotL / SHADOW_SLOPE_MIN_COS));

	return clamp(log2(footprint / texel * sqrt(slope)) + SHADOW_LOD_BIAS + ShadowSlots[slot].params.x, 0.0, SHADOW_MIPS - 1.0);
}

bool IsShadowMipBlending(float lod) {
	return SHADOW_DITHER_SIZE > 0.0 && frac(lod) > 1.0 - SHADOW_MIP_BLEND;
}

uint2 GetShadowTexel(float3 dir, uint face, uint mip) {
	uint size = SHADOW_VIRTUAL_SIZE >> mip;
	return min(uint2(saturate(GetShadowFaceUV(dir, face)) * size), uint2(size - 1u, size - 1u));
}

float GetShadowTexelSize(float axisDistance, uint mip) {
	return 2.0 * axisDistance / (SHADOW_VIRTUAL_SIZE >> mip);
}

uint2 GetShadowPhysTexel(uint phys, uint2 local) {
	return uint2(phys % SHADOW_POOL_PAGES, phys / SHADOW_POOL_PAGES) * SHADOW_PAGE_SIZE + local;
}
// ---------------

#endif