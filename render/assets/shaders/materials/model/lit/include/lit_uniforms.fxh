#ifndef INCLUDED_LIT_UNIFORMS_GUARD
#define INCLUDED_LIT_UNIFORMS_GUARD

#include <structs.fxh>

cbuffer Constants {
    Camera g_Camera;
    Model g_Model;

    // CLUSTERED --
    float2 g_ClusterSize;
    float2 g_zNearFarVec;
    // ------------
};

#endif
