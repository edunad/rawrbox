#ifndef INCLUDED_DECAL_UNIFORMS
    #define INCLUDED_DECAL_UNIFORMS

    #ifdef DECAL_CONSTANTS
        struct DecalConstantsStruct {
            uint4 settings;
        };

        ConstantBuffer<DecalConstantsStruct> DecalConstants;
    #endif

    struct Decal {
        float4x4 worldToLocal;
        uint4 data;
        float4 color;
    };

    #define TOTAL_DECALS DecalConstants.settings.x
#endif
