#ifndef INCLUDED_LIGHT_UNIFORMS
#define INCLUDED_LIGHT_UNIFORMS

cbuffer LightConstants {
    // Light ---------
    uint4 g_LightSettings;
    // ------

    // Sun ----
    float4 g_SunDirection;
    float4 g_SunColor;
    // ----

    // Ambient ---
    float4 g_AmbientColor;
    // -----

    // Fog ---
    float4 g_FogColor;
    float4 g_FogSettings;
    // -----
};

struct Light {
    float4 position;
    float4 direction;
    float4 intensity;

    // -------
    float radius;
    float penumbra;
    float umbra;

    uint type;
    // -------
};

// Aka sun
struct DirectionalLight {
    float4 direction;
    float4 radiance;
};

#define LIGHT_POINT 1
#define LIGHT_SPOT 2
#define LIGHT_UNIFORMS

#endif
