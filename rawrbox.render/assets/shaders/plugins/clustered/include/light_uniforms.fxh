#ifndef INCLUDED_LIGHT_UNIFORMS
#define INCLUDED_LIGHT_UNIFORMS

cbuffer LightConstants {
    // Light ---------
    uint4 g_LightSettings;
    // ------

    // Ambient ---
    float4 g_AmbientColor;
    // -----

    // Fog ---
    float4 g_FogColor;
    float4 g_FogSettings;
    // -----

    // GRID ---
    float4  g_LightGridParams;
    // -----
};

struct Light {
    float4 position;
    float4 direction;

    float3 color;
    float  intensity;

    // -------
    float  radius;
    float  penumbra;
    float  umbra;

    uint   type;
    // -------
};

// Aka sun
struct DirectionalLight {
    float4 direction;
    float4 radiance;
};


#define LIGHT_POINT       1
#define LIGHT_SPOT        2
#define LIGHT_DIRECTIONAL 3

#define TOTAL_LIGHTS g_LightSettings.y
#define LIGHT_UNIFORMS
#endif
