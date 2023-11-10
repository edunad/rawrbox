
#ifndef THREAD_GROUP_SIZE
#   define THREAD_GROUP_SIZE 512
#endif

#ifndef CLUSTERS_X_THREADS
#   define CLUSTERS_X_THREADS 16
#endif

#ifndef CLUSTERS_Y_THREADS
#   define CLUSTERS_Y_THREADS 8
#endif

#ifndef CLUSTERS_Z_THREADS
#   define CLUSTERS_Z_THREADS 4
#endif

#ifndef CLUSTERS_X
#   define CLUSTERS_X 16
#endif

#ifndef CLUSTERS_Y
#   define CLUSTERS_Y 8
#endif

#ifndef CLUSTERS_Z
#   define CLUSTERS_Z 24
#endif

#ifndef CLUSTER_COUNT
#   define CLUSTER_COUNT 3072
#endif


#ifndef INCLUDED_CLUSTER_UNIFORMS
#define INCLUDED_CLUSTER_UNIFORMS

cbuffer Constants {
    // CAMERA ------
    float4   g_ScreenSize;
    float4x4 g_InvProj;
    float4x4 g_View;
    // --------------

    float2   g_ClusterSize;
    float2   g_zNearFarVec;
};

struct Cluster {
    float3 minBounds;
    float3 maxBounds;
};

#endif
