
$input v_normal, v_tangent, v_bitangent, v_texcoord0, v_color0, v_worldPos

#define READ_MATERIAL

#include <bgfx_shader.sh>

#include "../../include/clusters.sh"
#include "../../include/lights.sh"
#include "../../include/defs.sh"

SAMPLER2D(s_albedo, SAMPLE_MAT_ALBEDO);
SAMPLER2D(s_normal, SAMPLE_MAT_NORMAL);
SAMPLER2D(s_specular, SAMPLE_MAT_SPECULAR);

uniform vec4 u_colorOffset;

vec2 blinn(vec3 _lightDir, vec3 _normal, vec3 _viewDir) {
	float ndotl = dot(_normal, _lightDir);
	vec3 reflected = _lightDir - 2.0 * ndotl * _normal;
	float rdotv = dot(reflected, _viewDir);
	return vec2(ndotl, rdotv);
}

vec4 lit(float _ndotl, float _rdotv, float _m) {
	float diff = max(0.0, _ndotl);
	float spec = step(0.0, _ndotl) * max(0.0, _rdotv * _m);
	return vec4(1.0, diff, spec, 1.0);
}

void main() {
	vec4 albedo = texture2D(s_albedo, v_texcoord0) * v_color0 * u_colorOffset;

	float specular = texture2D(s_specular, v_texcoord0).r;
	albedo.a = specular;

	// ----
	vec3 normal;
	normal.xy = texture2D(s_normal, v_texcoord0).xy * 2.0 - 1.0;
	normal.z  = sqrt(1.0 - dot(normal.xy, normal.xy) );

	mat3 tbn = mat3(
				normalize(v_tangent),
				normalize(v_bitangent),
				normalize(v_normal)
				);

	normal = normalize(mul(tbn, normal));
	vec3 wnormal = normalize(mul(u_invView, vec4(normal, 0.0)).xyz);
	// -----

    vec3 fragPos = v_worldPos;

	// Apply lights ----
	if(u_fullbright == 0.0) {
		vec3 radianceOut = vec3_splat(0.0);

		uint cluster = getClusterIndex(gl_FragCoord);
		LightGrid grid = getLightGrid(cluster);

		for(uint i = 0; i < grid.pointLights; i++) {
			uint lightIndex = getGridLightIndex(grid.offset, i);
			PointLight light = getPointLight(lightIndex);

			float dist = distance(light.position, fragPos);
			float attenuation = smoothAttenuation(dist, light.radius);
			if(attenuation > 0.0) {
				radianceOut += light.intensity * attenuation;
			}
		}

		radianceOut += getAmbientLight().irradiance;
		gl_FragColor.rgb = radianceOut * albedo;
	} else {
		gl_FragColor = albedo;
	}
	// -------
    gl_FragColor.a = albedo.a;
}
