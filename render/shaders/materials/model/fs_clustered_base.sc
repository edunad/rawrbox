
$input v_normal, v_tangent, v_texcoord0, v_color0, v_worldPos

#define READ_LIGHT_INDICES
#define READ_LIGHT_GRID

#include <bgfx_shader.sh>

#include "../../include/clusters.sh"
#include "../../include/lights.sh"

SAMPLER2D(s_albedo, SAMPLE_MAT_ALBEDO);
SAMPLER2D(s_normal, SAMPLE_MAT_NORMAL);
SAMPLER2D(s_specular, SAMPLE_MAT_SPECULAR);
SAMPLER2D(s_emission, SAMPLE_MAT_EMISSION);
SAMPLER2D(s_opacity, SAMPLE_MAT_OPACITY);

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

uniform vec4 u_texMatData;
uniform vec4 u_specularColor;
uniform vec4 u_emissionColor;

void main() {
	vec4 albedo = texture2D(s_albedo, v_texcoord0.xy) * v_color0 * u_colorOffset;
	float specular = texture2D(s_specular, v_texcoord0.xy).r;
	vec4 emission = texture2D(s_emission, v_texcoord0.xy) * v_color0 * u_colorOffset;
	float opacity = texture2D(s_opacity, v_texcoord0.xy).r;

	// ----
	vec3 normal;
	normal.xy = texture2D(s_normal, v_texcoord0.xy).xy * 2.0 - 1.0;
	normal.z  = sqrt(1.0 - dot(normal.xy, normal.xy));

    vec3 modelNormal = convertTangentNormal(v_normal, v_tangent, normal);
	// -----

    vec3 fragPos = v_worldPos;
    vec3 viewDir = normalize(u_camPos.xyz - v_worldPos);

	// Apply lights ----
	if(u_fullbright == 0.0) {
		vec3 radianceOut = vec3_splat(0.0);

		uint cluster = getClusterIndex(gl_FragCoord);
		LightGrid grid = getLightGrid(cluster);

		// Point lights ----
		for(uint i = 0; i < grid.pointLights; i++) {
			uint lightIndex = getGridLightIndex(grid.offset, i);
			PointLight light = getPointLight(lightIndex);

			float dist = distance(light.position, fragPos);
			float attenuation = smoothAttenuation(dist, light.radius);

			// Diffuse shading
			vec3 lightDirection = normalize(light.position - fragPos);
			vec3 reflectDir = reflect(-lightDirection, modelNormal);
			vec3 spec = specular * (pow(max(dot(reflectDir, viewDir), 0.0), u_texMatData.x)* attenuation);

			if(attenuation > 0.0) {
				radianceOut += light.intensity * attenuation;
			}
		}
		// --------

		// Sun ----
		/*DirectionalLight sun = getSunLight();
		if(sun.radiance.r != 0.0 && sun.radiance.g != 0.0 && sun.radiance.b != 0.0) {
			radianceOut += sun.radiance * max(dot(modelNormal, -sun.direction), 0.0f);
		}*/
		// --------

		// Final ----
		radianceOut += getAmbientLight();
		radianceOut += emission;
		gl_FragColor.rgb = radianceOut * albedo;
		// --------
	} else {
		gl_FragColor = albedo;
	}
	// -------

    gl_FragColor.a = albedo.a * opacity;
}
