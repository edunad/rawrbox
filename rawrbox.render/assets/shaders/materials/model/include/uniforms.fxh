#ifndef INCLUDED_UNIFORMS_GUARD
#define INCLUDED_UNIFORMS_GUARD

#include <model.fxh>
#include <camera.fxh>

cbuffer Constants {
    Model g_Model;
    #ifdef SKINNED
        float4x4 g_Bones[MAX_BONES];
    #endif
};
#endif
