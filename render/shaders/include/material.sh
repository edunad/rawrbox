#ifndef DEFS_MATERIAL_HEADER_GUARD
#define DEFS_MATERIAL_HEADER_GUARD

#include "defs.sh"

#ifdef VERTEX_DATA
    #define MAX_DATA 4

    uniform vec4 u_data[MAX_DATA];

    #define billboard u_data[0].xyz
    #define vertexSnap u_data[1].x
    #define displacement_power u_data[2].x
    #define recieve_decals u_data[3].x
#endif

#ifdef READ_G_BUFFER
    SAMPLER2D(s_texDiffuse,           DEFERRED_DIFFUSE);
    SAMPLER2D(s_texNormal,            DEFERRED_NORMAL);
    SAMPLER2D(s_texEmissionSpec,      DEFERRED_EMISSION_SPEC);
    SAMPLER2D(s_texBitMask,           DEFERRED_BITMASK);
    SAMPLER2D(s_texDepth,             DEFERRED_DEPTH);
#endif

#ifdef READ_LIGHT

#endif

#endif // DEFS_MATERIAL_HEADER_GUARD
