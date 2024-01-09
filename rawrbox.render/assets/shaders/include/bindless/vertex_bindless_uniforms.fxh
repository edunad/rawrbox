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
    #define DisplacementPower Constants.data[2].x
    #define RecieveDecals Constants.data[3].x

    ConstantBuffer<ConstantsStruct> Constants;
#endif
