#ifndef INCLUDED_PIXEL_POST_PROCESS_UNIFORMS
    #define INCLUDED_PIXEL_POST_PROCESS_UNIFORMS

    struct PostProcessConstantsStruct {
        #ifdef UINT_DATA
        uint4 data[MAX_POST_DATA];
        #else
        float4 data[MAX_POST_DATA];
        #endif

        uint textureID;
        uint textureDepthID;
    };

    ConstantBuffer<PostProcessConstantsStruct> PostProcessConstants;
#endif
