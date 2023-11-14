#ifndef INCLUDED_CLUSTER
#define INCLUDED_CLUSTER

#include <cluster_macros.fxh>
#include <utils.fxh>

struct Cluster {
    float3 minBounds;
    float3 maxBounds;
};

#if defined(WRITE_CLUSTERS)
RWStructuredBuffer<Cluster> g_Clusters;
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
RWBuffer<uint2 /*format=r32i*/> g_ClusterGrid; // Read-Write
#elif defined(READ_CLUSTER_DATA_GRID)
#define CLUSTER_DATA_GRID
Buffer<uint2 /*format=r32i*/> g_ClusterGrid; // Read-only
#endif

// light indices belonging to clusters
#if defined(WRITE_LIGHT_INDICES)
RWBuffer<uint /*format=r32i*/> g_ClusterLightIndices; // Read-Write
#define LIGHT_INDICES
#elif defined(READ_LIGHT_INDICES)
Buffer<uint /*format=r32i*/> g_ClusterLightIndices; // Read-only
#define LIGHT_INDICES
#endif


// atomic index
#if defined(WRITE_ATOMIC)
RWBuffer<uint /*format=r32i*/> g_AtomicIndex; // Read-Write
#define ATOMIC
#elif defined(READ_ATOMIC)
Buffer<uint /*format=r32i*/> g_AtomicIndex; // Read-only
#define ATOMIC
#endif


#ifdef CLUSTER_DATA_GRID
struct ClusterDataGrid {
    uint offset;
    uint lights;
};

#ifdef READ_CLUSTER_DATA_GRID
ClusterDataGrid getClusterDataGrid(uint cluster) {
    uint2 gridvec = g_ClusterGrid[cluster];

    ClusterDataGrid grid;
    grid.offset = gridvec.x;
    grid.lights = gridvec.y;

    return grid;
}
#endif

#endif

#ifdef LIGHT_INDICES
uint getGridLightIndex(uint start, uint offset) {
    return g_ClusterLightIndices[start + offset];
}
#endif

#ifdef CLUSTER_UNIFORMS
// cluster depth index from depth in screen coordinates (gl_FragCoord.z)
uint getClusterZIndex(float screenDepth) {
    // this can be calculated on the CPU and passed as a uniform
    // only leaving it here to keep most of the relevant code in the shaders for learning purposes
    float scale = float(CLUSTERS_Z) / log(g_Cluster.zNearFarVec.y / g_Cluster.zNearFarVec.x);
    float bias = -(float(CLUSTERS_Z) * log(g_Cluster.zNearFarVec.x) / log(g_Cluster.zNearFarVec.y / g_Cluster.zNearFarVec.x));

    float eyeDepth = screen2EyeDepth(screenDepth, g_Cluster.zNearFarVec.x, g_Cluster.zNearFarVec.y);
    uint zIndex = uint(max(log(eyeDepth) * scale + bias, 0.0));

    return zIndex;
}

uint getClusterIndex(float4 position) {
    uint zIndex = getClusterZIndex(position.z);
    uint3 indices = uint3(uint2(position.xy / g_Cluster.clusterSize), zIndex);

    return (CLUSTERS_X * CLUSTERS_Y) * indices.z + CLUSTERS_X * indices.y + indices.x;
}
#endif

#endif
