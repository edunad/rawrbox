#ifndef INCLUDED_MODEL
    #define INCLUDED_MODEL

    struct Model {
        float4 colorOverride;
        float4 textureFlags;
        float4 data[4];
    };

    #define Billboard Constants.model.data[0]
    #define VertexSnap Constants.model.data[1].x
    #define DisplacementPower Constants.model.data[2].x
    #define RecieveDecals Constants.model.data[3].x
#endif
