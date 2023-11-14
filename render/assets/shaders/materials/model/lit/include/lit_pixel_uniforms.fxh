#ifndef INCLUDED_LIT_PIXEL_UNIFORMS_GUARD
#define INCLUDED_LIT_PIXEL_UNIFORMS_GUARD

#include <cluster_structs.fxh>

cbuffer Constants {
    // CLUSTERED --
    ClusterData g_Cluster;
    // ------------

    // CAMERA ----
    float3 g_CameraPos;
    // -----------

    // Textures ----
    float  g_SpecularPower;
    // -------------

};

#define CLUSTER_UNIFORMS
#endif
