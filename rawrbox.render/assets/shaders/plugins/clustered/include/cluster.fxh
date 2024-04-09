#ifndef INCLUDED_CLUSTER
#define INCLUDED_CLUSTER

struct ClusterAABB {
	float4 Center;
	float4 Extents;
};

struct ClusterData {
	uint lights;
	uint decals;
	uint2 __offset;
};

#if defined(WRITE_CLUSTERS)
RWStructuredBuffer<ClusterAABB> Clusters; // Read-Write
	#define CLUSTERS
#elif defined(READ_CLUSTERS)
StructuredBuffer<ClusterAABB> Clusters;        // Read-only
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
#endif
