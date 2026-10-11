// TODO: Replace with MeshShaders
#include "shadow_uniforms.fxh"

StructuredBuffer<uint> ShadowPages;
StructuredBuffer<uint> ShadowRects;

#define SHADOW_CULL_PAGES 16u

struct GSInput {
	float4 Pos : SV_POSITION;
	float3 Dir : POSITION1;
	float2 UV : TEX_COORD;

	nointerpolation uint View : VIEW;
};

struct PSInput {
	float4 Pos : SV_POSITION;
	float3 Dir : POSITION1;
	float2 UV : TEX_COORD;

	nointerpolation uint View : VIEW;
	uint Viewport : SV_ViewportArrayIndex;
};

bool HasPages(uint slot, uint face, uint mip, uint layer, uint2 first, uint2 last) {
	if ((last.x - first.x + 1u) * (last.y - first.y + 1u) > SHADOW_CULL_PAGES) return true;

	uint flag = layer == SHADOW_LAYER_STATIC ? SHADOW_DRAW : SHADOW_ALLOCATED;
	for (uint y = first.y; y <= last.y; y++) {
		for (uint x = first.x; x <= last.x; x++) {
			uint2 page = mip == 0u ? GetShadowTablePage(slot, face, uint2(x, y)) : uint2(x, y);
			if ((ShadowPages[GetShadowEntry(slot, face, mip, page)] & flag) != 0u) return true;
		}
	}

	return false;
}

// Build shadow
[maxvertexcount(3)]
void main(triangle GSInput input[3], inout TriangleStream<PSInput> triangles) {
	uint view = input[0].View;
	uint slot = GetShadowViewSlot(view);
	uint face = GetShadowViewFace(view);
	uint mip = GetShadowViewMip(view);
	uint layer = GetShadowViewLayer(view);

	uint base = (GetShadowBatch(slot, face, layer) * SHADOW_MIPS + mip) * 4u;
	uint4 rect = uint4(ShadowRects[base], ShadowRects[base + 1u], ShadowRects[base + 2u], ShadowRects[base + 3u]);
	if (rect.x > rect.z) return;

	float size = float(SHADOW_VIRTUAL_SIZE >> mip);
	float4 bounds = float4(rect.xy, rect.zw + 1u) * SHADOW_PAGE_SIZE;

	// Skip missing triangles
	bool crossing = false;

	float2 low = size;
	float2 high = 0.0;

	for (uint i = 0; i < 3; i++) {
		float4 pos = input[i].Pos;

		if (pos.w <= 0.0) {
			crossing = true;
			continue;
		}

		float2 texel = float2(pos.x / pos.w * 0.5 + 0.5, 0.5 - pos.y / pos.w * 0.5) * size;
		low = min(low, texel);
		high = max(high, texel);
	}

	if (!crossing) {
		if (any(high < bounds.xy) || any(low > bounds.zw)) return;

		uint2 first = max((uint2)max(low, 0.0) / SHADOW_PAGE_SIZE, rect.xy);
		uint2 last = min((uint2)max(high, 0.0) / SHADOW_PAGE_SIZE, rect.zw);
		if (!HasPages(slot, face, mip, layer, first, last)) return;
	}
	// -----------

	[unroll]
	for (uint v = 0; v < 3; v++) {
		PSInput output;

		output.Pos = input[v].Pos;
		output.Dir = input[v].Dir;
		output.UV = input[v].UV;
		output.View = input[v].View;
		output.Viewport = mip;

		triangles.Append(output);
	}
}