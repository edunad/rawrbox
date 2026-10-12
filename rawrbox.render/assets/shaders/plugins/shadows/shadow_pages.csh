#include "shadow_uniforms.fxh"

StructuredBuffer<uint2> ShadowInvalidations;
StructuredBuffer<uint> ShadowBatchRects;
StructuredBuffer<uint4> ShadowDrawRecords;

RWStructuredBuffer<uint> ShadowPageTableRW;
RWStructuredBuffer<uint> ShadowRequests;
RWStructuredBuffer<uint> ShadowPhysOwner;
RWStructuredBuffer<uint> ShadowPhysClear;
RWStructuredBuffer<uint> ShadowTouched;
RWStructuredBuffer<uint> ShadowFreeStack;
RWStructuredBuffer<uint> ShadowAllocator;
RWStructuredBuffer<uint> ShadowAges;
RWStructuredBuffer<uint> ShadowClearList;

RWByteAddressBuffer ShadowClearArgs;

RWStructuredBuffer<uint> ShadowViewsRW;
RWStructuredBuffer<uint> ShadowRectsRW;
RWStructuredBuffer<uint> ShadowBatches;
RWStructuredBuffer<uint> ShadowFeedback;
RWStructuredBuffer<uint> ShadowFaceRects;

RWByteAddressBuffer ShadowDrawArgs;

RWTexture2DArray<uint> ShadowPoolRW;

#define ALLOCATOR_DEMAND  4u
#define ALLOCATOR_RANKS   (4u + SHADOW_MIPS)
#define ALLOCATOR_PENDING (4u + SHADOW_MIPS * 2u)
#define ALLOCATOR_DRAWS   (4u + SHADOW_MIPS * 3u)

#define DRAW_ARGS_STRIDE 20u

groupshared uint EvictCutoff;
groupshared uint EvictRemain;

// https://ktstephano.github.io/rendering/stratusgfx/svsm

// UTILS ---
bool NeedsPage(uint index) {
	return ShadowRequests[index] + 1u == SHADOW_FRAME && (ShadowPageTableRW[index] & SHADOW_ALLOCATED) == 0u;
}

void ReleasePage(uint phys, uint owner, uint stack) {
	ShadowPageTableRW[owner] = 0u;
	ShadowPhysOwner[phys] = SHADOW_EMPTY;
	ShadowFreeStack[stack] = phys;
}

uint GetFreePages() {
	return ShadowAllocator[0] + ShadowAges[1];
}

uint GetEntryMip(uint index) {
	uint local = index % SHADOW_FACE_ENTRIES;
	for (uint mip = 1u; mip < SHADOW_MIPS; mip++) {
		if (local < GetShadowMipOffset(mip)) return mip - 1u;
	}

	return SHADOW_MIPS - 1u;
}

uint2 GetEntryPage(uint index, uint mip) {
	uint pages = SHADOW_PAGES >> mip;
	uint local = index % SHADOW_FACE_ENTRIES - GetShadowMipOffset(mip);
	uint2 page = uint2(local % pages, local / pages);

	return mip == 0u ? GetShadowWindowPage(index / SHADOW_LIGHT_ENTRIES, (index / SHADOW_FACE_ENTRIES) % 6u, page) : page;
}

bool IsUnusedMip(uint index, uint mip) {
	return mip != 0u && IsShadowClipmap(index / SHADOW_LIGHT_ENTRIES);
}

bool IsRecorded(uint index, uint mip) {
	if (IsUnusedMip(index, mip)) return false;

	uint face = index / SHADOW_FACE_ENTRIES;
	uint2 page = GetEntryPage(index, mip);

	return ShadowRectsOverlap(UnpackShadowRect(ShadowBatchRects[face * 2u + SHADOW_LAYER_STATIC]), uint4(page << mip, ((page + 1u) << mip) - 1u));
}

uint GetCoarseRank(uint counter, uint totals, uint mip) {
	uint rank;
	InterlockedAdd(ShadowAllocator[counter + mip], 1u, rank);

	for (uint coarser = mip + 1u; coarser < SHADOW_MIPS; coarser++) {
		rank += ShadowAllocator[totals + coarser];
	}

	return rank;
}

void KeepCoarsePage(uint index) {
	if (index % SHADOW_FACE_ENTRIES != SHADOW_FACE_ENTRIES - 1u || ShadowSlots[index / SHADOW_LIGHT_ENTRIES].params.y < 0.5) return;
	ShadowRequests[index] = SHADOW_FRAME;
}

void AddView(uint slot, uint face, uint mip, uint layer, uint2 page) {
	uint batch = GetShadowBatch(slot, face, layer);
	uint rect = (batch * SHADOW_MIPS + mip) * 4u;

	InterlockedMin(ShadowRectsRW[rect + 0u], page.x);
	InterlockedMin(ShadowRectsRW[rect + 1u], page.y);
	InterlockedMax(ShadowRectsRW[rect + 2u], page.x);
	InterlockedMax(ShadowRectsRW[rect + 3u], page.y);

	uint live;
	InterlockedOr(ShadowBatches[batch * 2u + 1u], 1u << mip, live);
	if ((live & (1u << mip)) != 0u) return;

	uint view;
	InterlockedAdd(ShadowBatches[batch * 2u], 1u, view);
	ShadowViewsRW[batch * SHADOW_MIPS + view] = PackShadowView(slot, face, mip, layer);
}
// ---------

[numthreads(64, 1, 1)]
void Invalidate(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID) {
	if (groupId.x >= SHADOW_INVALIDATIONS) return;

	uint2 record = ShadowInvalidations[groupId.x];
	uint slot = GetShadowViewSlot(record.x);
	uint face = record.x >> SHADOW_SLOT_BITS;
	uint4 rect = uint4(record.y & 0xFFu, (record.y >> 8u) & 0xFFu, (record.y >> 16u) & 0xFFu, record.y >> 24u);

	uint mips = IsShadowClipmap(slot) ? 1u : SHADOW_MIPS;
	for (uint mip = 0; mip < mips; mip++) {
		uint4 mipRect = rect >> mip;
		uint width = mipRect.z - mipRect.x + 1u;
		uint count = width * (mipRect.w - mipRect.y + 1u);

		for (uint i = groupThreadId.x; i < count; i += 64u) {
			uint2 page = uint2(mipRect.x + i % width, mipRect.y + i / width);
			uint index = GetShadowEntry(slot, face, mip, mip == 0u ? GetShadowTablePage(slot, face, page) : page);
			uint entry = ShadowPageTableRW[index];
			if ((entry & SHADOW_ALLOCATED) == 0u) continue;

			ShadowPageTableRW[index] = entry & ~SHADOW_RENDERED;
			InterlockedOr(ShadowPhysClear[entry & SHADOW_PHYS_MASK], SHADOW_CLEAR_STATIC);
		}
	}
}

[numthreads(64, 1, 1)]
void Free(uint3 id : SV_DispatchThreadID) {
	uint phys = id.x;
	uint owner = phys < SHADOW_TOTAL_PAGES ? ShadowPhysOwner[phys] : SHADOW_EMPTY;

	if (owner != SHADOW_EMPTY) {
		uint age = SHADOW_FRAME - ShadowRequests[owner];

		if (age > SHADOW_PAGE_CACHE) {
			uint stack;
			InterlockedAdd(ShadowAllocator[0], 1u, stack);
			ReleasePage(phys, owner, stack);
		} else if (age >= 2u) {
			InterlockedAdd(ShadowAges[age], 1u);
		}
	}

	if (id.x < SHADOW_ACTIVE_SLOTS * SHADOW_LIGHT_ENTRIES && NeedsPage(id.x)) {
		InterlockedAdd(ShadowAllocator[1], 1u);
		InterlockedAdd(ShadowAllocator[ALLOCATOR_DEMAND + GetEntryMip(id.x)], 1u);
	}

	// Reset ---
	uint batches = SHADOW_ACTIVE_SLOTS * 12u;
	if (id.x < batches * SHADOW_MIPS) {
		ShadowRectsRW[id.x * 4u + 0u] = 0xFFFFu;
		ShadowRectsRW[id.x * 4u + 1u] = 0xFFFFu;
		ShadowRectsRW[id.x * 4u + 2u] = 0u;
		ShadowRectsRW[id.x * 4u + 3u] = 0u;
	}

	if (id.x < batches) {
		ShadowBatches[id.x * 2u] = 0u;
		ShadowBatches[id.x * 2u + 1u] = 0u;
	}

	if (id.x < SHADOW_ACTIVE_SLOTS * 6u) {
		ShadowFeedback[id.x] = 0u;

		ShadowFaceRects[id.x * 4u + 0u] = 0xFFFFu;
		ShadowFaceRects[id.x * 4u + 1u] = 0xFFFFu;
		ShadowFaceRects[id.x * 4u + 2u] = 0u;
		ShadowFaceRects[id.x * 4u + 3u] = 0u;
	}
	// -----------
}

[numthreads(64, 1, 1)]
void Evict(uint3 id : SV_DispatchThreadID, uint3 groupThreadId : SV_GroupThreadID) {
	if (groupThreadId.x == 0u) {
		uint free = ShadowAllocator[0];
		uint demand = ShadowAllocator[1];

		EvictCutoff = 0xFFFFFFFFu;
		EvictRemain = 0u;

		if (demand > free) {
			uint deficit = demand - free;
			uint total = 0u;

			EvictCutoff = 1u;
			for (uint age = SHADOW_PAGE_CACHE; age >= 2u; age--) {
				uint count = ShadowAges[age];
				if (total + count >= deficit) {
					EvictCutoff = age;
					EvictRemain = deficit - total;
					break;
				}

				total += count;
			}
		}
	}

	GroupMemoryBarrierWithGroupSync();

	uint phys = id.x;
	if (phys >= SHADOW_TOTAL_PAGES) return;

	uint owner = ShadowPhysOwner[phys];
	if (owner == SHADOW_EMPTY) return;

	uint age = SHADOW_FRAME - ShadowRequests[owner];
	if (age < 2u || age < EvictCutoff) return;

	if (age == EvictCutoff) {
		uint evicted;
		InterlockedAdd(ShadowAges[0], 1u, evicted);
		if (evicted >= EvictRemain) return;
	}

	uint stack;
	InterlockedAdd(ShadowAges[1], 1u, stack);
	ReleasePage(phys, owner, ShadowAllocator[0] + stack);
}

[numthreads(64, 1, 1)]
void Allocate(uint3 id : SV_DispatchThreadID) {
	uint index = id.x;
	if (index >= SHADOW_ACTIVE_SLOTS * SHADOW_LIGHT_ENTRIES) return;

	uint mip = GetEntryMip(index);
	uint entry = ShadowPageTableRW[index];

	if (ShadowRequests[index] + 1u == SHADOW_FRAME) {
		uint face = index / SHADOW_FACE_ENTRIES;
		uint2 page = GetEntryPage(index, mip);

		InterlockedMin(ShadowFaceRects[face * 4u + 0u], page.x << mip);
		InterlockedMin(ShadowFaceRects[face * 4u + 1u], page.y << mip);
		InterlockedMax(ShadowFaceRects[face * 4u + 2u], ((page.x + 1u) << mip) - 1u);
		InterlockedMax(ShadowFaceRects[face * 4u + 3u], ((page.y + 1u) << mip) - 1u);
	}

	if ((entry & SHADOW_ALLOCATED) != 0u) {
		if ((entry & SHADOW_RENDERED) == 0u && IsRecorded(index, mip))
		    InterlockedAdd(ShadowAllocator[ALLOCATOR_PENDING + mip], 1u);
		return;
	}

	if (!NeedsPage(index)) return;

	uint free = GetFreePages();
	uint rank = GetCoarseRank(ALLOCATOR_RANKS, ALLOCATOR_DEMAND, mip);
	if (rank >= free) return;

	uint phys = ShadowFreeStack[free - 1u - rank];

	ShadowPageTableRW[index] = phys | SHADOW_ALLOCATED;
	ShadowPhysOwner[phys] = index;

	InterlockedOr(ShadowPhysClear[phys], SHADOW_CLEAR_STATIC | SHADOW_CLEAR_DYNAMIC);
	if (IsRecorded(index, mip)) InterlockedAdd(ShadowAllocator[ALLOCATOR_PENDING + mip], 1u);
}

[numthreads(64, 1, 1)]
void Select(uint3 id : SV_DispatchThreadID) {
	uint index = id.x;
	if (index >= SHADOW_ACTIVE_SLOTS * SHADOW_LIGHT_ENTRIES) return;

	uint entry = ShadowPageTableRW[index];
	if ((entry & SHADOW_ALLOCATED) == 0u) return;

	uint slot = index / SHADOW_LIGHT_ENTRIES;
	uint face = (index / SHADOW_FACE_ENTRIES) % 6u;

	uint mip = GetEntryMip(index);
	if (IsUnusedMip(index, mip)) return;

	uint2 page = GetEntryPage(index, mip);

	AddView(slot, face, mip, SHADOW_LAYER_DYNAMIC, page);
	if ((entry & SHADOW_RENDERED) != 0u) return;

	bool selected = IsRecorded(index, mip);
	if (selected && SHADOW_RENDER_BUDGET != 0u)
	    selected = GetCoarseRank(ALLOCATOR_DRAWS, ALLOCATOR_PENDING, mip) < SHADOW_RENDER_BUDGET;

	if (!selected) {
		ShadowFeedback[slot * 6u + face] = 1u;
		return;
	}

	ShadowPageTableRW[index] = entry | SHADOW_DRAW;
	AddView(slot, face, mip, SHADOW_LAYER_STATIC, page);
}

[numthreads(64, 1, 1)]
void Patch(uint3 id : SV_DispatchThreadID) {
	if (id.x >= SHADOW_DRAWS) return;

	uint4 record = ShadowDrawRecords[id.x];
	uint views = ShadowBatches[record.x * 2u];
	uint4 mesh = UnpackShadowRect(record.w);

	// CULL
	bool canReach = false;
	for (uint k = 0; k < views && !canReach; k++) {
		if (record.y != SHADOW_ALL_VIEWS && record.y != k) continue;

		uint mip = GetShadowViewMip(ShadowViewsRW[record.x * SHADOW_MIPS + k]);
		uint rect = (record.x * SHADOW_MIPS + mip) * 4u;
		uint4 pages = uint4(ShadowRectsRW[rect], ShadowRectsRW[rect + 1u], ShadowRectsRW[rect + 2u], ShadowRectsRW[rect + 3u]);

		canReach = ShadowRectsOverlap(mesh >> mip, pages);
	}
	// -----------

	uint instances = !canReach ? 0u : (record.y == SHADOW_ALL_VIEWS ? views * record.z : record.z);
	ShadowDrawArgs.Store(id.x * DRAW_ARGS_STRIDE + 4u, instances);
}

[numthreads(64, 1, 1)]
void Prepare(uint3 id : SV_DispatchThreadID) {
	if (id.x == 0u) { // Reset
		uint free = GetFreePages();
		uint requested = ShadowAllocator[1];
		uint allocated = min(free, requested);

		ShadowAllocator[0] = free - allocated;
		ShadowAllocator[1] = 0u;
		ShadowAllocator[2] = requested - allocated;

		for (uint counter = 4u; counter < 4u + SHADOW_MIPS * 4u; counter++) {
			ShadowAllocator[counter] = 0u;
		}

		for (uint bin = 0u; bin <= SHADOW_PAGE_CACHE; bin++) {
			ShadowAges[bin] = 0u;
		}
	}

	uint phys = id.x;
	if (phys >= SHADOW_TOTAL_PAGES) return;

	uint flags = ShadowPhysClear[phys] | (ShadowTouched[phys] != 0u ? SHADOW_CLEAR_DYNAMIC : 0u);
	if (flags == 0u) return;

	ShadowPhysClear[phys] = 0u;
	ShadowTouched[phys] = 0u;

	uint index;
	ShadowClearArgs.InterlockedAdd(0, 1u, index);
	ShadowClearList[index] = phys | (flags << 16u);
}

[numthreads(8, 8, 1)]
void Clear(uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID) {
	uint item = ShadowClearList[groupId.x];
	uint flags = item >> 16u;
	uint2 origin = GetShadowPhysTexel(item & SHADOW_PHYS_MASK, uint2(0u, 0u));

	for (uint y = groupThreadId.y; y < SHADOW_PAGE_SIZE; y += 8u) {
		for (uint x = groupThreadId.x; x < SHADOW_PAGE_SIZE; x += 8u) {
			uint2 texel = origin + uint2(x, y);

			if ((flags & SHADOW_CLEAR_STATIC) != 0u) ShadowPoolRW[uint3(texel, SHADOW_LAYER_STATIC)] = SHADOW_EMPTY;
			if ((flags & SHADOW_CLEAR_DYNAMIC) != 0u) ShadowPoolRW[uint3(texel, SHADOW_LAYER_DYNAMIC)] = SHADOW_EMPTY;
		}
	}
}

[numthreads(64, 1, 1)]
void Resolve(uint3 id : SV_DispatchThreadID) {
	uint index = id.x;
	if (index >= SHADOW_ACTIVE_SLOTS * SHADOW_LIGHT_ENTRIES) return;

	KeepCoarsePage(index);

	uint entry = ShadowPageTableRW[index];
	if ((entry & SHADOW_DRAW) != 0u) ShadowPageTableRW[index] = (entry & ~SHADOW_DRAW) | SHADOW_RENDERED;
}

[numthreads(64, 1, 1)]
void Discard(uint3 id : SV_DispatchThreadID) {
	uint index = id.x;
	if (index >= SHADOW_ACTIVE_SLOTS * SHADOW_LIGHT_ENTRIES) return;

	KeepCoarsePage(index);

	uint entry = ShadowPageTableRW[index];
	if ((entry & SHADOW_DRAW) != 0u) ShadowPageTableRW[index] = entry & ~SHADOW_DRAW;
}