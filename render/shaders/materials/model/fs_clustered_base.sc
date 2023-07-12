
$input v_normal, v_tangent, v_texcoord0, v_color0, v_worldPos

#define READ_LIGHT_INDICES
#define READ_LIGHT_GRID

#include <bgfx_shader.sh>

#include "../../include/clusters.sh"
#include "../../include/lights.sh"

SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);
SAMPLER2DARRAY(s_normal, SAMPLE_MAT_NORMAL);
SAMPLER2DARRAY(s_specular, SAMPLE_MAT_SPECULAR);
SAMPLER2DARRAY(s_emission, SAMPLE_MAT_EMISSION);
SAMPLER2DARRAY(s_opacity, SAMPLE_MAT_OPACITY);

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

uniform vec4 u_texMatData;

void main() {
	vec4 albedo = texture2DArray(s_albedo, vec3(v_texcoord0.xy, v_texcoord0.z)) * v_color0 * u_colorOffset;
	if (albedo.a <= 0.0) discard;

	vec4 normal = texture2DArray(s_normal, vec3(v_texcoord0.xy, v_texcoord0.z));
	vec4 specular = texture2DArray(s_specular, vec3(v_texcoord0.xy, v_texcoord0.z)) * v_color0;
	vec4 emission = texture2DArray(s_emission, vec3(v_texcoord0.xy, v_texcoord0.z)) * v_color0 * u_colorOffset;
	vec4 opacity = texture2DArray(s_opacity, vec3(v_texcoord0.xy, v_texcoord0.z)).a;

	// ----
	vec3 norm = normalize(v_normal);
    vec3 modelNormal = convertTangentNormal(v_normal, v_tangent, normal);
	// -----

    vec3 viewDir = normalize(u_camPos - v_worldPos);

	// Apply lights ----
	if(u_fullbright == 0.0) {
		vec3 radianceOut = vec3_splat(0.0);

		uint cluster = getClusterIndex(gl_FragCoord);
		LightGrid grid = getLightGrid(cluster);

		// Point lights ----
		for(uint i = 0; i < grid.pointLights; i++) {
			uint lightIndex = getGridLightIndex(grid.offset, i);
			PointLight light = getPointLight(lightIndex);

			vec3 lightDir = normalize(light.position - v_worldPos);
			float dist = distance(light.position, v_worldPos);
			float attenuation = smoothAttenuation(dist, light.radius);

			if(attenuation > 0.0) {
				float NdotL = dot(norm, lightDir);
				if (NdotL > 0.0) {
					vec3 reflectDir = reflect(-lightDir, norm);
					float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_texMatData.x);

					radianceOut += light.intensity * spec * specular.r; // Specular
				}

				radianceOut += light.intensity * attenuation; // Diffuse
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
		radianceOut += emission;

		gl_FragColor.rgb = albedo * radianceOut;
		// --------
	} else {
		gl_FragColor.rgb = albedo;
	}
	// -------

    gl_FragColor.a = albedo.a * opacity;
}
