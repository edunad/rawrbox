#ifndef INCLUDED_CLUSTER
#define INCLUDED_CLUSTER

#include <utils.fxh>

struct Cluster {
    float4 minBounds;
    float4 maxBounds;
};

#if defined(WRITE_CLUSTERS)
RWStructuredBuffer<Cluster> g_Clusters; // Read-Write
#define CLUSTERS
#elif defined(READ_CLUSTERS)
StructuredBuffer<Cluster> g_Clusters; // Read-only
#define CLUSTERS
#endif

#if defined(READ_LIGHTS)
StructuredBuffer<Light> g_Lights; // Read-only
#define LIGHT
#endif

#if defined(WRITE_CLUSTER_DATA_GRID)
#define CLUSTER_DATA_GRID
RWBuffer<uint4 /*format=rgba32ui*/> g_ClusterDataGrid; // Read-Write
#elif defined(READ_CLUSTER_DATA_GRID)
#define CLUSTER_DATA_GRID
Buffer<uint4 /*format=rgba32ui*/> g_ClusterDataGrid; // Read-only
#endif

// light indices belonging to clusters
#if defined(WRITE_LIGHT_INDICES)
RWBuffer<uint /*format=r32ui*/> g_ClusterLightIndices; // Read-Write
#define LIGHT_INDICES
#elif defined(READ_LIGHT_INDICES)
Buffer<uint /*format=r32ui*/> g_ClusterLightIndices; // Read-only
#define LIGHT_INDICES
#endif


// atomic index
#if defined(WRITE_ATOMIC)
RWBuffer<uint /*format=r32ui*/> g_AtomicIndex; // Read-Write
#define ATOMIC
#elif defined(READ_ATOMIC)
Buffer<uint /*format=r32ui*/> g_AtomicIndex; // Read-only
#define ATOMIC
#endif


#ifdef READ_CLUSTER_DATA_GRID
uint4 GetClusterDataGrid(uint cluster) {
    return g_ClusterDataGrid[cluster];
}
#endif

#ifdef LIGHT_INDICES
uint GetGridLightIndex(uint start, uint offset) {
    return g_ClusterLightIndices[start + offset];
}
#endif

#ifdef CLUSTER_UNIFORMS
uint GetClusterZIndex(float depth, float2 camNearFar) {

    // TODO: pre-compute on cpu
    float scale = float(CLUSTERS_Z) / log2(camNearFar.y / camNearFar.x);
    float bias = -(float(CLUSTERS_Z) * log2(camNearFar.x) / log2(camNearFar.y / camNearFar.x));

    float linearDepth = GetLinearDepth(depth, camNearFar);
    uint zIndex = uint(max(log(linearDepth) * scale + bias, 0.0));

    return zIndex;
}

uint GetClusterIndex(float depth, float2 camNearFar, float2 clusterSize, float4 pos) {
    // // linear depth
    // float linearDepth = GetLinearDepth(depth, camNearFar);

    // // could be pre computed on cpu.
    // float scale = CLUSTERS_Z / log2(camNearFar.y / camNearFar.x);
    // float bias = -(CLUSTERS_Z * log2(camNearFar.x) / log2(camNearFar.y / camNearFar.x));

    // uint zTile = uint(max(log2(linearDepth) * scale + bias, 0.0));
    // uint3 tiles = uint3(uint2(pos.xy / (uint)clusterSize.x), zTile);

    // uint tileIndex = tiles.x + CLUSTERS_X * tiles.y + (CLUSTERS_X * CLUSTERS_Y) * tiles.z;
    // return tileIndex;

    uint zIndex = GetClusterZIndex(depth, camNearFar);
    uint3 indices = uint3(uint2(pos.xy / clusterSize.xy), zIndex);

    uint cluster = (CLUSTERS_X * CLUSTERS_Y) * indices.z +
                   CLUSTERS_X * indices.y +
                   indices.x;
    return cluster;

}
#endif

#endif
