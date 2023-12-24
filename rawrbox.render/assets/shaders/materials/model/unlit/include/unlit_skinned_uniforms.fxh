#ifndef INCLUDED_UNLIT_SKINNED_UNIFORMS_GUARD
#define INCLUDED_UNLIT_SKINNED_UNIFORMS_GUARD

#include <model.fxh>
#include <camera.fxh>

cbuffer Constants {
    Model g_Model;
    float4x4 g_Bones[MAX_BONES];
};

#endif
