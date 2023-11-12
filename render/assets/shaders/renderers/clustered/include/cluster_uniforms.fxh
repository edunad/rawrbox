#ifndef INCLUDED_CLUSTER_UNIFORMS
#define INCLUDED_CLUSTER_UNIFORMS

struct Camera {
    float4   screenSize;
    float4x4 invProj;
    float4x4 view;
};

cbuffer Constants {
    Camera   g_Camera;

    float2   g_ClusterSize;
    float2   g_zNearFarVec;
};

#define CAMERA_UNIFORMS
#endif
