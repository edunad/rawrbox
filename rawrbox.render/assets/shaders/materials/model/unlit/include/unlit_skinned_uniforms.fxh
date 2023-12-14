#ifndef INCLUDED_UNLIT_SKINNED_UNIFORMS_GUARD
#define INCLUDED_UNLIT_SKINNED_UNIFORMS_GUARD

#include <model_vertex_structs.fxh>

cbuffer Constants {
    Camera g_Camera;
    Model g_Model;

    float4x4 g_Bones[MAX_BONES];
};

#define CAMERA_UNIFORMS
#define MODEL_UNIFORMS
#endif
