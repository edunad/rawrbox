#ifndef INCLUDED_VERTEX_UNIFORMS
    #define INCLUDED_VERTEX_UNIFORMS

    struct ConstantsStruct {
        // Model ----
        float4 colorOverride;
        float4 textureFlags;
        float4 data[4];
        // ----------------
    };

    #define Billboard Constants.data[0]
    #define VertexSnap Constants.data[1].x
    #define DisplacementTexture Constants.data[2].x
    #define DisplacementPower Constants.data[2].y
    #define RecieveDecals Constants.data[3].x
    #define RecieveShadows Constants.data[3].y

    ConstantBuffer<ConstantsStruct> Constants;
#endif
