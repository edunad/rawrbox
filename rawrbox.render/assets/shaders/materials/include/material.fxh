#ifndef INCLUDED_MATERIAL_HEADER_GUARD
#define INCLUDED_MATERIAL_HEADER_GUARD

#ifdef TEXTURE_DATA
    #define UV_FLAG_NONE 0
    #define UV_FLAG_FLIP_U 1
    #define UV_FLAG_FLIP_V 2
    #define UV_FLAG_FLIP_UV 3

    float2 applyUVTransform(float2 uv) {
        if(Constants.textureFlags.x == UV_FLAG_FLIP_U) uv.x = -uv.x;
        if(Constants.textureFlags.x == UV_FLAG_FLIP_V) uv.y = -uv.y;
        if(Constants.textureFlags.x == UV_FLAG_FLIP_UV) uv.xy = -uv.xy;

        return uv;
    }
#endif

#endif // INCLUDED_MATERIAL_HEADER_GUARD

