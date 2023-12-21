#ifndef INCLUDED_MODEL
    #define INCLUDED_MODEL

    struct Model {
        float4 colorOverride;
        float4 textureFlags;
        float4 data[4];
    };

    #define Billboard g_Model.data[0]
    #define VertexSnap g_Model.data[1].x
    #define DisplacementPower g_Model.data[2].x
    #define RecieveDecals g_Model.data[3].x
#endif
