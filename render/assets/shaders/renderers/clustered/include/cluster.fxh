#ifndef INCLUDED_CLUSTER
#define INCLUDED_CLUSTER

#include <utils.fxh>

struct ClusterAABB {
	float4 Center;
	float4 Extents;
};

#if defined(WRITE_CLUSTERS)
RWStructuredBuffer<ClusterAABB> g_Clusters; // Read-Write
#define CLUSTERS
#elif defined(READ_CLUSTERS)
StructuredBuffer<ClusterAABB> g_Clusters; // Read-only
#define CLUSTERS
#endif

#if defined(READ_LIGHTS)
StructuredBuffer<Light> g_Lights; // Read-only
#define LIGHT
#endif

#if defined(WRITE_CLUSTER_DATA_GRID)
#define CLUSTER_DATA_GRID
RWBuffer<uint /*format=rgba32ui*/> g_ClusterDataGrid; // Read-Write
#elif defined(READ_CLUSTER_DATA_GRID)
#define CLUSTER_DATA_GRID
Buffer<uint /*format=rgba32ui*/> g_ClusterDataGrid; // Read-only
#endif

#endif
