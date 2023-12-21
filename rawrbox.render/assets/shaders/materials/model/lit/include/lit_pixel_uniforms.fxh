#ifndef INCLUDED_LIT_PIXEL_UNIFORMS_GUARD
#define INCLUDED_LIT_PIXEL_UNIFORMS_GUARD

cbuffer Constants {
    float4  g_LightGridParams;
    // --------
    float4  g_LitData;
};

#define SpecularPower g_LitData.x
#define EmissionPower g_LitData.y

#define CLUSTER_UNIFORMS
#endif
