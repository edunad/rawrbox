// Adapted from: https://github.com/pezcode/Cluster ♥

#ifndef LIGHTS_SH_HEADER_GUARD
#define LIGHTS_SH_HEADER_GUARD

#include <bgfx_compute.sh>
#include "defs.sh"

uniform vec4 u_lightSettings;
#define u_fullbright uint(u_lightSettings.x)
#define u_pointLightCount uint(u_lightSettings.y)

uniform vec4 u_ambientLight;
uniform vec4 u_sunDirection;
uniform vec4 u_sunColor;

// for each light:
//   vec4 position (w is padding)
//   vec4 intensity + radius (xyz is intensity, w is radius)
BUFFER_RO(b_pointLights, vec4, SAMPLE_LIGHTS_POINTLIGHTS);

struct PointLight {
    vec3 position;
    float _padding;
    vec3 intensity;
    float radius;
};

// Aka sun
struct DirectionalLight {
    vec3 direction;
    vec3 radiance;
};

// primary source:
// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
// also really good:
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float distanceAttenuation(float distance) {
    // only for point lights

    // physics: inverse square falloff
    // to keep irradiance from reaching infinity at really close distances, stop at 1cm
    return 1.0 / max(distance * distance, 0.01 * 0.01);
}

float smoothAttenuation(float distance, float radius) {
    float nom = saturate(1.0 - pow(distance / radius, 4.0));
    return nom * nom * distanceAttenuation(distance);
}

uint pointLightCount() {
    return u_pointLightCount;
}

PointLight getPointLight(uint i) {
    PointLight light;
    light.position = b_pointLights[2 * i + 0].xyz;

    vec4 intensityRadiusVec = b_pointLights[2 * i + 1];
    light.intensity = intensityRadiusVec.xyz;
    light.radius = intensityRadiusVec.w;

    return light;
}

DirectionalLight getSunLight() {
    DirectionalLight light;

    light.direction = u_sunDirection.xyz;
    light.radiance = u_sunColor.xyz;
    return light;
}

vec3 getAmbientLight() {
    return u_ambientLight.xyz;
}
#endif // LIGHTS_SH_HEADER_GUARD
