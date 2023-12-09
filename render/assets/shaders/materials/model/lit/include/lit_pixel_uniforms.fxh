#ifndef INCLUDED_LIT_PIXEL_UNIFORMS_GUARD
#define INCLUDED_LIT_PIXEL_UNIFORMS_GUARD

cbuffer Constants {
    float2  g_ZNearFarVec;
    float2  g_ClusterSize;
// --------
    float4  g_LitData;
    float4  g_CameraPos;
};

#define SpecularPower g_LitData.x
#define EmissionPower g_LitData.y

#define CLUSTER_UNIFORMS
#endif
