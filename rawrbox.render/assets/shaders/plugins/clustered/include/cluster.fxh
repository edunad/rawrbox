#ifndef INCLUDED_CLUSTER
#define INCLUDED_CLUSTER

#include "math.fxh"

struct ClusterAABB {
	float4 Center;
	float4 Extents;
};

struct ClusterData {
	uint lights;
	uint decals;
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
RWStructuredBuffer<ClusterData> ClusterDataGrid; // Read-Write
	#define CLUSTER_DATA_GRID
#elif defined(READ_CLUSTER_DATA_GRID)
StructuredBuffer<ClusterData> ClusterDataGrid; // Read-only
	#define CLUSTER_DATA_GRID
#endif

#define GROUP_SIZE uint3(CLUSTERS_X, CLUSTERS_Y, CLUSTERS_Z)

// UTILS -----
uint GetClusterBucketCount(uint total) {
	return min((total + CLUSTER_BUCKET_SIZE - 1) / CLUSTER_BUCKET_SIZE, CLUSTERED_NUM_BUCKETS);
}

#if defined(INCLUDED_CAMERA) && defined(CLUSTER_DATA_GRID)
uint GetClusterGridOffset(float4 svPosition) {
	uint2 tile = min(uint2(svPosition.xy / ScreenSize * float2(CLUSTERS_X, CLUSTERS_Y)), uint2(CLUSTERS_X - 1, CLUSTERS_Y - 1));
	uint slice = (uint)clamp(GetSliceFromDepth(svPosition.w), 0.0, CLUSTERS_Z - 1.0);

	return Flatten3D(uint3(tile, slice), uint2(CLUSTERS_X, CLUSTERS_Y)) * CLUSTERED_NUM_BUCKETS;
}
#endif
// -----------
#endif
