#ifndef INCLUDED_PIXEL_UNIFORMS
    #define INCLUDED_PIXEL_UNIFORMS
    struct ConstantsStruct {
        uint4 textureIDs;
    };

    ConstantBuffer<ConstantsStruct> Constants;
#endif
