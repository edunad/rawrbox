#ifndef INCLUDED_LIT_PIXEL_UNIFORMS
#define INCLUDED_LIT_PIXEL_UNIFORMS

#include <camera.fxh>

cbuffer Constants {
    float4  g_LitData;
};

#define RoughtnessPower   g_LitData.x
#define EmissionPower     g_LitData.y
#define MetalPower        g_LitData.z

#endif
