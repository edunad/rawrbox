// Adapted from: https://github.com/pezcode/Cluster ♥

#ifndef CLUSTERS_SH_HEADER_GUARD
#define CLUSTERS_SH_HEADER_GUARD

#include <bgfx_compute.sh>
#include "defs.sh"
#include "utils.sh"

// taken from Doom
// http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf
#define CLUSTERS_X 16
#define CLUSTERS_Y 8
#define CLUSTERS_Z 24

// workgroup size of the culling compute shader
// D3D compute shaders only allow up to 1024 threads per workgroup
// GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS also only guarantees 1024
#define CLUSTERS_X_THREADS 16
#define CLUSTERS_Y_THREADS 8
#define CLUSTERS_Z_THREADS 4

#define MAX_LIGHTS_PER_CLUSTER 100

uniform vec4 u_clusterSizesVec; // cluster size in screen coordinates (pixels)
uniform vec4 u_zNearFarVec;

#define u_clusterSizes u_clusterSizesVec.xy
#define u_zNear        u_zNearFarVec.x
#define u_zFar         u_zNearFarVec.y

// list of clusters (2 vec4's each, min + max pos for AABB)
#if defined(WRITE_CLUSTERS)
BUFFER_RW(b_clusters, vec4, SAMPLE_CLUSTERS);
#define CLUSTERS
#elif defined(READ_CLUSTERS)
BUFFER_RO(b_clusters, vec4, SAMPLE_CLUSTERS);
#define CLUSTERS
#endif

// light indices belonging to clusters
#if defined(WRITE_LIGHT_INDICES)
BUFFER_RW(b_clusterLightIndices, uint, SAMPLE_LIGHTINDICES);
#define LIGHT_INDICES
#elif defined(READ_LIGHT_INDICES)
BUFFER_RO(b_clusterLightIndices, uint, SAMPLE_LIGHTINDICES);
#define LIGHT_INDICES
#endif

// light indices belonging to clusters
#if defined(WRITE_ATOMIC)
BUFFER_RW(b_globalIndex, uint, SAMPLE_ATOMIC_INDEX);
#define ATOMIC
#elif defined(READ_ATOMIC)
BUFFER_RO(b_globalIndex, uint, SAMPLE_ATOMIC_INDEX);
#define ATOMIC
#endif

// for each cluster: (start index in b_clusterLightIndices, number of point lights, empty, empty)
#if defined(WRITE_LIGHT_GRID)
#define LIGHT_GRID
BUFFER_RW(b_clusterLightGrid, uvec4, SAMPLE_LIGHTGRID);
#elif defined(READ_LIGHT_GRID)
#define LIGHT_GRID
BUFFER_RO(b_clusterLightGrid, uvec4, SAMPLE_LIGHTGRID);
#endif


struct Cluster {
    vec3 minBounds;
    vec3 maxBounds;
};

struct LightGrid {
    uint offset;
    uint lights;
};


#ifdef CLUSTERS
Cluster getCluster(uint index) {
    Cluster cluster;
    cluster.minBounds = b_clusters[2 * index + 0].xyz;
    cluster.maxBounds = b_clusters[2 * index + 1].xyz;
    return cluster;
}
#endif

#ifdef LIGHT_GRID
LightGrid getLightGrid(uint cluster) {
    uvec4 gridvec = b_clusterLightGrid[cluster];
    LightGrid grid;
    grid.offset = gridvec.x;
    grid.lights = gridvec.y;
    return grid;
}
#endif

#ifdef LIGHT_INDICES
uint getGridLightIndex(uint start, uint offset) {
    return b_clusterLightIndices[start + offset];
}
#endif

// cluster depth index from depth in screen coordinates (gl_FragCoord.z)
uint getClusterZIndex(float screenDepth) {
    // this can be calculated on the CPU and passed as a uniform
    // only leaving it here to keep most of the relevant code in the shaders for learning purposes
    float scale = float(CLUSTERS_Z) / log(u_zFar / u_zNear);
    float bias = -(float(CLUSTERS_Z) * log(u_zNear) / log(u_zFar / u_zNear));

    float eyeDepth = screen2EyeDepth(screenDepth, u_zNear, u_zFar);
    uint zIndex = uint(max(log(eyeDepth) * scale + bias, 0.0));
    return zIndex;
}

// cluster index from fragment position in window coordinates (gl_FragCoord)
uint getClusterIndex(vec4 fragCoord) {
    uint zIndex = getClusterZIndex(fragCoord.z);
    uvec3 indices = uvec3(uvec2(fragCoord.xy / u_clusterSizes.xy), zIndex);
    uint cluster = (CLUSTERS_X * CLUSTERS_Y) * indices.z +
                   CLUSTERS_X * indices.y +
                   indices.x;
    return cluster;
}
#endif // CLUSTERS_SH_HEADER_GUARD
