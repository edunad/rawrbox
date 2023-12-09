#ifndef INCLUDED_LIT_UNIFORMS_GUARD
#define INCLUDED_LIT_UNIFORMS_GUARD

#include <model_vertex_structs.fxh>

cbuffer Constants {
    Camera  g_Camera;
    Model   g_Model;
    float2  g_ZNearFarVec;
};

#define CAMERA_UNIFORMS
#define MODEL_UNIFORMS
#define CLUSTER_UNIFORMS
#endif
