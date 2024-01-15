#ifndef INCLUDED_DECAL_UNIFORMS
    #define INCLUDED_DECAL_UNIFORMS

    struct DecalsConstantsStruct {
        uint4 settings;
    };

    ConstantBuffer<DecalsConstantsStruct> DecalsConstants;
    #define TOTAL_DECALS DecalsConstants.settings.x

    struct Decal {
        float4x4 worldToLocal;
        uint4 data;
        float4 color;
    };

#endif
