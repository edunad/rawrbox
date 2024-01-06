#ifndef INCLUDED_3DTEXT_UNLIT_PIXEL_UNIFORMS
#define INCLUDED_3DTEXT_UNLIT_PIXEL_UNIFORMS

struct ConstantsStruct {
    uint textureID;
};

ConstantBuffer<ConstantsStruct> Constants;
#endif
