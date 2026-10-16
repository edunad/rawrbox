#include "include/instance.fxh"

struct InstanceCullStruct {
	float4 clip[4];
	float4 center;
	float4 extents;

	uint4 settings;
	uint4 hiz;
};

ConstantBuffer<InstanceCullStruct> CullConstants;

RWStructuredBuffer<uint> VisibleInstances;
RWStructuredBuffer<uint> Visibility;

RWByteAddressBuffer DrawArgs;
Texture2D<float> HiZ;

void Append(uint draw, uint index, bool visible) {
	if (CullConstants.hiz.z != 0u) {
		DrawArgs.Store((draw * CullConstants.settings.x + index) * 20u + 4u, visible ? 1u : 0u);
		return;
	}

	if (!visible) return;

	uint slot;
	DrawArgs.InterlockedAdd(draw * 20u + 4u, 1u, slot);
	VisibleInstances[slot] = index;
}

[numthreads(CULL_THREADS, 1, 1)]
void main(uint3 id : SV_DispatchThreadID) {
	if (id.x >= CullConstants.settings.x) return;

	// EARLY SKIP ---
	bool late = CullConstants.settings.y != 0u;
	bool last = CullConstants.settings.z != 0u || Visibility[id.x] != 0u;

	if (!late && !last) {
		Append(0u, id.x, false);
		return;
	}
	// ------

	float3x4 world = GetInstanceMatrix(Instances[id.x]);

	// FRUSTUM CHECK ---
	float4 corners[8];
	uint outside = 0x3Fu;

	[unroll]
	for (uint corner = 0; corner < 8; corner++) {
		float3 local = CullConstants.center.xyz + CullConstants.extents.xyz * float3((corner & 1u) != 0u ? 1.0 : -1.0, (corner & 2u) != 0u ? 1.0 : -1.0, (corner & 4u) != 0u ? 1.0 : -1.0);
		float4 pos = float4(mul(world, float4(local, 1.0)), 1.0);
		float4 clip = float4(dot(CullConstants.clip[0], pos), dot(CullConstants.clip[1], pos), dot(CullConstants.clip[2], pos), dot(CullConstants.clip[3], pos));

		uint planes = (clip.x < -clip.w ? 1u : 0u) | (clip.x > clip.w ? 2u : 0u) | (clip.y < -clip.w ? 4u : 0u) | (clip.y > clip.w ? 8u : 0u) | (clip.z < -clip.w ? 16u : 0u) | (clip.z > clip.w ? 32u : 0u);
		outside &= planes;
		corners[corner] = clip;
	}

	bool visible = outside == 0u && (CullConstants.hiz.z == 0u || Instances[id.x].data.w == 0u);
	// ------

	if (!late) {
		Append(0u, id.x, visible);
		return;
	}

	// HI-Z ---
	uint mips = CullConstants.settings.w;
	if (visible && mips != 0u) {
		float2 ndcMin = 1e30;
		float2 ndcMax = -1e30;
		float depth = 1.0;
		bool crossing = false;

		[unroll]
		for (uint corner = 0; corner < 8; corner++) {
			float4 clip = corners[corner];
			crossing = crossing || clip.w <= 1e-5;

			float2 ndc = clip.xy / max(clip.w, 1e-5);
			ndcMin = min(ndcMin, ndc);
			ndcMax = max(ndcMax, ndc);
			depth = min(depth, clip.z / max(clip.w, 1e-5));
		}

		if (!crossing) {
			// 3x3 MIP check ---
			int2 size = int2(CullConstants.hiz.xy);
			int2 pixelMin = min(int2(saturate(float2(ndcMin.x, -ndcMax.y) * 0.5 + 0.5) * float2(size)), size - 1);
			int2 pixelMax = min(int2(saturate(float2(ndcMax.x, -ndcMin.y) * 0.5 + 0.5) * float2(size)), size - 1);

			int2 extent = pixelMax - pixelMin + 1;
			uint mip = uint(clamp(int(ceil(log2(float(max(extent.x, extent.y))))) - 2, 0, int(mips) - 1));

			int2 dims = max(size >> (mip + 1u), 1);
			int2 texelMin = min(pixelMin >> (mip + 1u), dims - 1);
			int2 texelMax = min(pixelMax >> (mip + 1u), dims - 1);
			// ------

			float far = 0.0;
			for (int y = texelMin.y; y <= texelMax.y; y++) {
				for (int x = texelMin.x; x <= texelMax.x; x++) {
					far = max(far, HiZ.Load(int3(x, y, int(mip))));
				}
			}

			visible = depth <= far;
		}
	}
	// ------

	Visibility[id.x] = visible ? 1u : 0u;
	Append(1u, id.x, visible && !last);
}