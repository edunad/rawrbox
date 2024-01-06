#ifndef INCLUDED_UNIFORMS_GUARD
#define INCLUDED_UNIFORMS_GUARD

#include <model.fxh>
#include <camera.fxh>

struct ConstantsStruct {
    Model model;

    #ifdef SKINNED
        float4x4 bones[MAX_BONES];
    #endif
};

ConstantBuffer<ConstantsStruct> Constants;
#endif
