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

    uint type;
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
    return 1.0 / max(distance * distance, 0.01 * 0.01);
}

float smoothAttenuation(float distance, float radius) {
    float nom = saturate(1.0 - pow(distance / radius, 4.0));
    return nom * nom * distanceAttenuation(distance);
}

uint totalLights() {
    return u_lightCount;
}

vec4 getLightData(int id, int index) {
	return b_lights[id * 3 + index];
}

Light getLight(uint i) {
    Light light;

    vec4 posOuter = getLightData(i, 0);
    light.position = posOuter.xyz;
    light.outerCone = posOuter.w;

    vec4 intensityRadiusVec = getLightData(i, 1);
    light.intensity = intensityRadiusVec.xyz;
    light.radius = intensityRadiusVec.w;

    vec4 dirInner = getLightData(i, 2);
    light.direction = dirInner.xyz;
    light.innerCone = dirInner.w;

    light.type = dirInner.w == 0.0 ? LIGHT_POINT : LIGHT_SPOT;

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

#if defined(READ_LIGHT_INDICES) && defined(READ_LIGHT_GRID)
vec3 applyLight(vec4 fragCoord, vec3 worldPos, vec3 norm) {
	if(u_fullbright == 0.0) {
		vec3 radianceOut = vec3_splat(0.0);

		uint cluster = getClusterIndex(fragCoord);
		LightGrid grid = getLightGrid(cluster);

		// Lights ----
		for(uint i = 0; i < grid.lights; i++) {
			uint lightIndex = getGridLightIndex(grid.offset, i);
			Light light = getLight(lightIndex);

			vec3 lightDir = normalize(light.position - worldPos);
			float dist = distance(light.position, worldPos);

			// Spotlight
			if(light.type == LIGHT_SPOT) {
				float theta = dot(light.direction, -lightDir);

				if(theta > light.innerCone) {
					radianceOut += light.intensity * clamp((theta - light.innerCone) / (light.outerCone - light.innerCone), 0.0f, 1.0f); // Diffuse
				}
			} else { // Point light
				float attenuation = smoothAttenuation(dist, light.radius);

				if(attenuation > 0.0) {
					radianceOut += light.intensity * attenuation; // Diffuse
				}
			}
		}
		// --------

		// Sun ----
		DirectionalLight sun = getSunLight();
		if(sun.radiance.r != 0.0 && sun.radiance.g != 0.0 && sun.radiance.b != 0.0) {
			radianceOut += sun.radiance * max(dot(norm, -sun.direction), 0.0f); // Diffuse
		}
		// --------

		// Final ----
		radianceOut += getAmbientLight();
		// --------

    	return radianceOut;
	} else {
		return vec3_splat(1.0);
	}
}

vec3 applyLight(vec4 fragCoord, vec3 worldPos, vec3 norm, vec3 viewDir, float specular, float reflection) {
    if(u_fullbright == 0.0) {
		vec3 radianceOut = vec3_splat(0.0);

		uint cluster = getClusterIndex(fragCoord);
		LightGrid grid = getLightGrid(cluster);

		// Lights ----
		for(uint i = 0; i < grid.lights; i++) {
			uint lightIndex = getGridLightIndex(grid.offset, i);
			Light light = getLight(lightIndex);

			vec3 lightDir = normalize(light.position - worldPos);
			float dist = distance(light.position, worldPos);

			float NdotL = dot(norm, lightDir);

			// Spotlight
			if(light.type == LIGHT_SPOT) {
				float theta = dot(light.direction, -lightDir);

				if(theta > light.innerCone) {
					float intensity = clamp((theta - light.innerCone) / (light.outerCone - light.innerCone), 0.0f, 1.0f);

					if (NdotL > 0.0 && reflection > 0.0) {
						vec3 reflectDir = reflect(-lightDir, norm);
						float spec = pow(max(dot(viewDir, reflectDir), 0.0), reflection);
						radianceOut += light.intensity * spec * specular; // Specular
					}

					radianceOut += light.intensity * intensity; // Diffuse
				}
			} else { // Point light
				float attenuation = smoothAttenuation(dist, light.radius);

				if(attenuation > 0.0) {
					if (NdotL > 0.0 && reflection > 0.0) {
						vec3 reflectDir = reflect(-lightDir, norm);
						float spec = pow(max(dot(viewDir, reflectDir), 0.0), reflection);
						radianceOut += light.intensity * spec * specular; // Specular
					}

					radianceOut += light.intensity * attenuation; // Diffuse
				}
			}
		}
		// --------

		// Sun ----
		DirectionalLight sun = getSunLight();
		if(sun.radiance.r != 0.0 && sun.radiance.g != 0.0 && sun.radiance.b != 0.0) {
			radianceOut += sun.radiance * max(dot(norm, -sun.direction), 0.0f); // Diffuse
		}
		// --------

		// Final ----
		radianceOut += getAmbientLight();
		// --------

    	return radianceOut;
	} else {
		return vec3_splat(1.0);
	}
}

#endif

#endif // LIGHTS_SH_HEADER_GUARD
