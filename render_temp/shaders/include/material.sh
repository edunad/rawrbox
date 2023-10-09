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

#ifdef TEXTURE_DATA
    uniform vec4 u_tex_flags;
    #define uvFlags u_tex_flags.x

    #define UV_FLAG_NONE 0.0
    #define UV_FLAG_FLIP_U 1.0
    #define UV_FLAG_FLIP_V 2.0
    #define UV_FLAG_FLIP_UV 3.0

    vec2 applyUVTransform(vec2 uv) {
        if(uvFlags == UV_FLAG_FLIP_U) uv.x = -uv.x;
        if(uvFlags == UV_FLAG_FLIP_V) uv.y = -uv.y;
        if(uvFlags == UV_FLAG_FLIP_UV) uv.xy = -uv.xy;

        return uv;
    }
#endif


#ifdef READ_MATERIAL
    SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);
    SAMPLER2DARRAY(s_normal, SAMPLE_MAT_NORMAL);
    SAMPLER2DARRAY(s_specular, SAMPLE_MAT_SPECULAR);
    SAMPLER2DARRAY(s_emission, SAMPLE_MAT_EMISSION);

    uniform vec4 u_texMatData;
    #define specularPower u_texMatData.x
#endif

#ifdef READ_DEPTH
    SAMPLER2D(s_depth, SAMPLE_DEPTH);
#endif

#ifdef READ_MASK
    SAMPLER2D(s_mask, SAMPLE_MASK);
#endif

#endif // DEFS_MATERIAL_HEADER_GUARD
