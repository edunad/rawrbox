#ifndef DEFS_MATERIAL_HEADER_GUARD
#define DEFS_MATERIAL_HEADER_GUARD

#define g_Billboard g_Data[0]
#define g_VertexSnap g_Data[1].x
#define g_DisplacementPower g_Data[2].x
#define g_RecieveDecals g_Data[3].x

#ifdef TEXTURE_DATA
    #define UV_FLAG_NONE 0.0
    #define UV_FLAG_FLIP_U 1.0
    #define UV_FLAG_FLIP_V 2.0
    #define UV_FLAG_FLIP_UV 3.0

    float2 applyUVTransform(float2 uv) {
        if(g_TextureFlags.x == UV_FLAG_FLIP_U) uv.x = -uv.x;
        if(g_TextureFlags.x == UV_FLAG_FLIP_V) uv.y = -uv.y;
        if(g_TextureFlags.x == UV_FLAG_FLIP_UV) uv.xy = -uv.xy;

        return uv;
    }
#endif

//
//
//#ifdef READ_MATERIAL
//    SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);
//    SAMPLER2DARRAY(s_normal, SAMPLE_MAT_NORMAL);
//    SAMPLER2DARRAY(s_specular, SAMPLE_MAT_SPECULAR);
//    SAMPLER2DARRAY(s_emission, SAMPLE_MAT_EMISSION);
//
//    uniform vec4 u_texMatData;
//    #define specularPower u_texMatData.x
//#endif
//
//#ifdef READ_DEPTH
//    SAMPLER2D(s_depth, SAMPLE_DEPTH);
//#endif
//
//#ifdef READ_MASK
//    SAMPLER2D(s_mask, SAMPLE_MASK);
//#endif
#endif // DEFS_MATERIAL_HEADER_GUARD

