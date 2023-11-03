#ifndef INCLUDED_CLUSTER_CONSTANTS
#define INCLUDED_CLUSTER_CONSTANTS

cbuffer Constants {
    // CAMERA ------
    float4   g_ScreenSize;
    float4x4 g_InvProj;
    // --------------

    float2 g_ClusterSize;
    float2 g_zNearFarVec;
};

struct Cluster {
    float4 minBounds;
    float4 maxBounds;
};

#endif
