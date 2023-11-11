#ifndef INCLUDED_MATERIAL_HEADER_GUARD
#define INCLUDED_MATERIAL_HEADER_GUARD

#ifdef TEXTURE_DATA
    #define UV_FLAG_NONE 0.0
    #define UV_FLAG_FLIP_U 1.0
    #define UV_FLAG_FLIP_V 2.0
    #define UV_FLAG_FLIP_UV 3.0

    float2 applyUVTransform(float2 uv) {
        if(g_Model.textureFlags.x == UV_FLAG_FLIP_U) uv.x = -uv.x;
        if(g_Model.textureFlags.x == UV_FLAG_FLIP_V) uv.y = -uv.y;
        if(g_Model.textureFlags.x == UV_FLAG_FLIP_UV) uv.xy = -uv.xy;

        return uv;
    }
#endif

#endif // INCLUDED_MATERIAL_HEADER_GUARD

