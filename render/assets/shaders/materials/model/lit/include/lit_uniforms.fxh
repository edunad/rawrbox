#ifndef INCLUDED_LIT_UNIFORMS_GUARD
#define INCLUDED_LIT_UNIFORMS_GUARD

#include <cluster_structs.fxh>
#include <structs.fxh>

cbuffer Constants {
    Camera g_Camera;
    Model g_Model;
    ClusterData g_Cluster;
};

#define CAMERA_UNIFORMS
#define MODEL_UNIFORMS
#define CLUSTER_UNIFORMS
#endif
