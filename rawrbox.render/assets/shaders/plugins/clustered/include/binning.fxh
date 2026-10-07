#ifndef INCLUDED_BINNING
#define INCLUDED_BINNING

// Z-binning
// Based on Drobot Improved Culling for Tiled and Clustered Rendering

#include "math.fxh"

// BUFFERS -----
#if defined(READ_LIGHTS)
StructuredBuffer<Light> Lights; // Read-only
#endif

#if defined(READ_DECALS)
StructuredBuffer<Decal> Decals; // Read-only, sorted by depth
#endif

#if defined(WRITE_TILES)
RWStructuredBuffer<uint> LightTiles; // Read-Write
RWStructuredBuffer<uint> DecalTiles; // Read-Write

StructuredBuffer<float4> LightBounds; // Read-only
StructuredBuffer<float4> DecalBounds; // Read-only
#elif defined(READ_TILES)
StructuredBuffer<uint> LightTiles; // Read-only
StructuredBuffer<uint> DecalTiles; // Read-only

StructuredBuffer<uint> LightZBins; // Read-only
StructuredBuffer<uint> DecalZBins; // Read-only
#endif
// -------------

// UTILS -----
uint2 UnpackZBin(uint value) {
	return uint2(value & 0xFFFFu, value >> 16u);
}

uint GetBinMask(uint bin, uint2 range) {
	uint low = (bin == range.x / BIN_BITS) ? range.x % BIN_BITS : 0u;
	uint high = (bin == range.y / BIN_BITS) ? range.y % BIN_BITS : BIN_BITS - 1u;

	return (0xFFFFFFFFu << low) & (0xFFFFFFFFu >> (31u - high));
}

#ifdef INCLUDED_CAMERA
uint GetTileIndex(float2 pixel) {
	uint2 tile = min(uint2(pixel / ScreenSize * float2(TILES_X, TILES_Y)), uint2(TILES_X - 1, TILES_Y - 1));
	return tile.x + tile.y * TILES_X;
}

uint GetZBin(float viewDepth) {
	float t = (viewDepth - NearFar.x) / max(NearFar.y - NearFar.x, 1e-4);
	return (uint)clamp(t * ZBINS, 0.0, ZBINS - 1.0);
}
#endif
// -----------

#endif
