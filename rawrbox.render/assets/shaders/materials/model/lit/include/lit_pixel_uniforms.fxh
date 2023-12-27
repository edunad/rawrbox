#ifndef INCLUDED_LIT_PIXEL_UNIFORMS
#define INCLUDED_LIT_PIXEL_UNIFORMS

#include <camera.fxh>

cbuffer Constants {
    float4  g_LitData;
};

#define RoughnessFactor    g_LitData.x
#define MetalnessFactor    g_LitData.y
#define SpecularFactor     g_LitData.z
#define EmissionFactor     g_LitData.w

#endif
