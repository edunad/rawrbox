// Adapted from: https://github.com/pezcode/Cluster ♥

#ifndef LIGHTS_SH_HEADER_GUARD
#define LIGHTS_SH_HEADER_GUARD

#include <bgfx_compute.sh>
#include "defs.sh"

uniform vec4 u_lightSettings;
#define u_fullbright uint(u_lightSettings.x)
#define u_lightCount uint(u_lightSettings.y)

uniform vec4 u_ambientLight;
uniform vec4 u_sunDirection;
uniform vec4 u_sunColor;

// for each light:
//   vec4 position (w is outerCone)
//   vec4 intensity + radius (xyz is intensity, w is radius)
//   vec4 direction + innerCone (xyz is direction, w is innerCone)
BUFFER_RO(b_lights, vec4, SAMPLE_LIGHTS);

struct Light {
    vec3 position;
    float outerCone;

    vec3 intensity;
    float radius;

    vec3 direction;
    float innerCone;
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

uint totalLights() {
    return u_lightCount;
}

Light getLight(uint i) {
    Light light;

    vec4 posOuter = b_lights[3 * i + 0];
    light.position = posOuter.xyz;
    light.outerCone = posOuter.w;

    vec4 intensityRadiusVec = b_lights[3 * i + 1];
    light.intensity = intensityRadiusVec.xyz;
    light.radius = intensityRadiusVec.w;

    vec4 dirInner = b_lights[3 * i + 2];
    light.direction = dirInner.xyz;
    light.innerCone = dirInner.w;

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
