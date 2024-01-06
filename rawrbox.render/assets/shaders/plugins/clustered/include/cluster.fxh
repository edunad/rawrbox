#ifndef INCLUDED_CLUSTER
	#define INCLUDED_CLUSTER

	struct ClusterAABB {
		float4 Center;
		float4 Extents;
	};

	#if defined(WRITE_CLUSTERS)
		RWStructuredBuffer<ClusterAABB> Clusters; // Read-Write
		#define CLUSTERS
	#elif defined(READ_CLUSTERS)
		StructuredBuffer<ClusterAABB> Clusters; // Read-only
		#define CLUSTERS
	#endif

	#if defined(READ_LIGHTS)
		StructuredBuffer<Light> Lights; // Read-only
		#define LIGHT
	#endif

	#if defined(READ_DECALS)
		StructuredBuffer<Decal> Decals; // Read-only
		#define DECALS
	#endif

	#if defined(WRITE_CLUSTER_DATA_GRID)
		#define CLUSTER_DATA_GRID
		RWStructuredBuffer<uint2 /*format=rgba32ui*/> ClusterDataGrid; // Read-Write
	#elif defined(READ_CLUSTER_DATA_GRID)
		#define CLUSTER_DATA_GRID
		StructuredBuffer<uint2 /*format=rgba32ui*/> ClusterDataGrid; // Read-only
	#endif

    // UTILS -----------------
	uint Flatten2D(uint2 index, uint dimensionsX) {
		return index.x + index.y * dimensionsX;
	}

	uint Flatten3D(uint3 index, uint2 dimensionsXY) {
		return index.x + index.y * dimensionsXY.x + index.z * dimensionsXY.x * dimensionsXY.y;
	}

	uint2 UnFlatten2D(uint index, uint dimensionsX) {
		return uint2(index % dimensionsX, index / dimensionsX);
	}

	uint3 UnFlatten3D(uint index, uint2 dimensionsXY) {
		uint3 outIndex;
		outIndex.z = index / (dimensionsXY.x * dimensionsXY.y);
		index -= (outIndex.z * dimensionsXY.x * dimensionsXY.y);
		outIndex.y = index / dimensionsXY.x;
		outIndex.x = index % dimensionsXY.x;
		return outIndex;
	}
    // -----------------------
#endif
