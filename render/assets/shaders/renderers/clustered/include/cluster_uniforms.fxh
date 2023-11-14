#ifndef INCLUDED_CLUSTER_UNIFORMS
#define INCLUDED_CLUSTER_UNIFORMS

#include <cluster_structs.fxh>

struct Camera {
    float4   screenSize;
    float4x4 invProj;
    float4x4 view;
};

cbuffer Constants {
    ClusterData g_Cluster;
    Camera      g_Camera;
};

#define CAMERA_UNIFORMS
#endif
