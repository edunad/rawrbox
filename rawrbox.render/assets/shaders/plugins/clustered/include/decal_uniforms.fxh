#ifndef INCLUDED_DECAL_UNIFORMS
    #define INCLUDED_DECAL_UNIFORMS

    #ifdef DECAL_CONSTANTS
        cbuffer DecalConstants {
            uint4 g_DecalSettings;
        };
    #endif

    struct Decal {
        float4x4 worldToLocal;
        uint4 data;
        float4 color;
    };

    #define TOTAL_DECALS g_DecalSettings.x
#endif
