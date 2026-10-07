#include "camera.fxh"
#include "decal_uniforms.fxh"
#include "light_uniforms.fxh"
#include "math.fxh"

#define WRITE_TILES
#include "binning.fxh"

struct TileFrustum {
	float3 planes[4];
};

TileFrustum GetTileFrustum(uint2 tile) {
	float2 tileSize = ScreenSize / float2(TILES_X, TILES_Y);
	float2 invScreen = 1.0 / ScreenSize;

	float2 minSS = float2(tile) * tileSize;
	float2 maxSS = float2(tile + 1) * tileSize;

	float3 corners[4];
	corners[0] = ScreenToView(float3(minSS.x, minSS.y, 1.0), invScreen, NearFar, SCamera.projInv);
	corners[1] = ScreenToView(float3(maxSS.x, minSS.y, 1.0), invScreen, NearFar, SCamera.projInv);
	corners[2] = ScreenToView(float3(maxSS.x, maxSS.y, 1.0), invScreen, NearFar, SCamera.projInv);
	corners[3] = ScreenToView(float3(minSS.x, maxSS.y, 1.0), invScreen, NearFar, SCamera.projInv);

	float3 center = corners[0] + corners[1] + corners[2] + corners[3];

	TileFrustum frustum;
	for (uint i = 0; i < 4; i++) {
		float3 plane = normalize(cross(corners[i], corners[(i + 1) % 4]));
		frustum.planes[i] = dot(plane, center) < 0.0 ? -plane : plane;
	}

	return frustum;
}

bool SphereInTile(float4 viewSphere, TileFrustum frustum) {
	for (uint i = 0; i < 4; i++) {
		if (dot(frustum.planes[i], viewSphere.xyz) < -viewSphere.w) return false;
	}

	return true;
}

groupshared float3 gPlanes[4];

[numthreads(TILE_THREADS, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID) {
	uint tileIndex = groupId.x;
	bool decals = groupId.z != 0;
	uint bin = groupId.y * TILE_THREADS + groupThreadId.x;

	// Build the tile frustum once per group ---
	if (groupThreadId.x == 0) {
		TileFrustum tileFrustum = GetTileFrustum(uint2(tileIndex % TILES_X, tileIndex / TILES_X));
		for (uint p = 0; p < 4; p++) {
			gPlanes[p] = tileFrustum.planes[p];
		}
	}

	GroupMemoryBarrierWithGroupSync();
	// ----

	uint total = decals ? TOTAL_DECALS : TOTAL_LIGHTS;
	uint first = bin * BIN_BITS;
	if (first >= total) return;

	TileFrustum frustum;
	for (uint p = 0; p < 4; p++) {
		frustum.planes[p] = gPlanes[p];
	}

	uint count = min(total - first, BIN_BITS);
	uint mask = 0;

	for (uint i = 0; i < count; i++) {
		float4 sphere = decals ? DecalBounds[first + i] : LightBounds[first + i];
		if (sphere.w < 0.0) continue; // Skip directional

		if (SphereInTile(sphere, frustum)) mask |= 1u << i;
	}

	if (decals) { // TODO: Add bin type?
		DecalTiles[tileIndex * DECAL_BINS + bin] = mask;
	} else {
		LightTiles[tileIndex * LIGHT_BINS + bin] = mask;
	}
}
