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
    float3 position;
    float outerCone;
    // -------
    float3 intensity;
    float radius;
    // -------
    float3 direction;
    float innerCone;
    // -------
    uint type;
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
