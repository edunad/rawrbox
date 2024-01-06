#ifndef INCLUDED_LIT_PIXEL_UNIFORMS
#define INCLUDED_LIT_PIXEL_UNIFORMS

#include <camera.fxh>

struct ConstantsStruct {
    uint4  textureIDs;
    float4 litData;
};

ConstantBuffer<ConstantsStruct> Constants;

#define RoughnessFactor    Constants.litData.x
#define MetalnessFactor    Constants.litData.y
#define SpecularFactor     Constants.litData.z
#define EmissionFactor     Constants.litData.w

#endif
