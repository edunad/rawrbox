#ifndef INCLUDED_DECAL_UNIFORMS
    #define INCLUDED_DECAL_UNIFORMS

    struct DecalsConstantsStruct {
        uint total;
    };

    ConstantBuffer<DecalsConstantsStruct> DecalsConstants;

    struct Decal {
        float4x4 worldToLocal;
        uint4 data;
        float4 color;
    };

#endif
