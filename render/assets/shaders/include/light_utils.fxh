#ifndef INCLUDED_LIGHT_UTILS
#define INCLUDED_LIGHT_UTILS

// primary source:
// https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
// also really good:
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float distanceAttenuation(float distance) {
    return 1.0 / max(distance * distance, 0.01 * 0.01);
}

float smoothAttenuation(float distance, float radius) {
    float nom = saturate(1.0 - pow(abs(distance / radius), 4.0));
    return nom * nom * distanceAttenuation(distance);
}

uint totalLights() {
    return g_LightSettings.y;
}

Light getLight(int id) {
    return g_Lights[id];
}

DirectionalLight getSunLight() {
    DirectionalLight light;
    light.direction = g_SunDirection;
    light.radiance = g_SunColor;

    return light;
}

float3 getAmbientLight() {
    return g_AmbientColor;
}

float3 applyLight(float4 fragCoord, float3 worldPos, float3 norm, float3 viewDir, float specular, float reflection) {
    if(g_LightSettings.x == 0.0) {
		/*float3 radianceOut = float3(0.0);

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

    	return radianceOut;*/

		return float3(2.0, 1.0, 1.0);
	} else {
		return float3(1.0, 1.0, 1.0);
	}
}


#endif
