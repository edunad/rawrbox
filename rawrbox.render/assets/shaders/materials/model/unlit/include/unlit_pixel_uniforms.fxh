#ifndef INCLUDED_UNLIT_PIXEL_UNIFORMS
#define INCLUDED_UNLIT_PIXEL_UNIFORMS

struct ConstantsStruct {
    uint4 textureIDs;
};

ConstantBuffer<ConstantsStruct> Constants;
#endif
