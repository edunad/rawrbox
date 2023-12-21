#ifndef INCLUDED_UNLIT_SKINNED_UNIFORMS_GUARD
#define INCLUDED_UNLIT_SKINNED_UNIFORMS_GUARD

#include <camera.fxh>
#include <model_vertex_structs.fxh>

cbuffer Constants {
    Model g_Model;
    float4x4 g_Bones[MAX_BONES];
};

#endif
