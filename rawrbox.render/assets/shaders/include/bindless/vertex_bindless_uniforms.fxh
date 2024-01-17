#ifndef INCLUDED_VERTEX_UNIFORMS
    #define INCLUDED_VERTEX_UNIFORMS

    struct ConstantsStruct {
        // Model ----
        float4 colorOverride;
        float4 textureFlags;
        float4 data[MAX_VERTEX_DATA];
        // ----------------

        // Model Bones ----
        float4x4 bones[MAX_BONES_PER_MODEL];
        // ----------------
    };

    ConstantBuffer<ConstantsStruct> Constants;

    #define Billboard               Constants.data[0]
    #define VertexSnap              Constants.data[1].x
    #define DisplacementTexture     Constants.data[2].x
    #define DisplacementPower       Constants.data[2].y
    #define RecieveDecals           Constants.data[3].x
    #define RecieveShadows          Constants.data[3].y
#endif
