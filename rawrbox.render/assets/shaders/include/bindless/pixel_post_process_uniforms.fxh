#ifndef INCLUDED_PIXEL_POST_PROCESS_UNIFORMS
    #define INCLUDED_PIXEL_POST_PROCESS_UNIFORMS

    struct PostProcessConstantsStruct {
        float4 data[2];

        uint textureID;
        uint textureDepthID;
    };

    ConstantBuffer<PostProcessConstantsStruct> PostProcessConstants;
#endif
