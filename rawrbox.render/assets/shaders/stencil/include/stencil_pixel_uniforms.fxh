#ifndef INCLUDED_STENCIL_PIXEL_UNIFORMS
#define INCLUDED_STENCIL_PIXEL_UNIFORMS

struct ConstantsStruct {
    uint textureID;
};

ConstantBuffer<ConstantsStruct> Constants;
#endif
