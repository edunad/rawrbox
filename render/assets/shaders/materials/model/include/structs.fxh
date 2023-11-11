#ifndef INCLUDED_STRUCTS_GUARD
#define INCLUDED_STRUCTS_GUARD

struct Camera {
    float4x4 model;
    float4x4 viewProj;
    float4x4 invView;
    float4x4 invProj;
    float4x4 worldViewProj;
    float4   screenSize;
};

struct Model {
    float4   colorOverride;
    float4   textureFlags;
    float4   data[4];
};

#define Billboard g_Model.data[0]
#define VertexSnap g_Model.data[1].x
#define DisplacementPower g_Model.data[2].x
#define RecieveDecals g_Model.data[3].x

#endif
