#ifndef INCLUDED_DECAL_UNIFORMS
    #define INCLUDED_DECAL_UNIFORMS

    struct DecalsConstantsStruct {
        uint total;
    };

    ConstantBuffer<DecalsConstantsStruct> DecalsConstants;
    #define TOTAL_DECALS DecalsConstants.total

    struct Decal {
        float4x4 worldToLocal;
        uint4 data;
        float4 color;
    };

#endif
