
$input v_normal, v_tangent, v_texcoord, v_gpuPick, v_color0, v_worldPos, v_data

#define READ_LIGHT_INDICES
#define READ_LIGHT_GRID
#define READ_MATERIAL

#include <bgfx_shader.sh>

#include "../../include/clusters.sh"
#include "../../include/lights.sh"
#include "../../include/model_transforms.sh"
#include "../../include/material.sh"
#include "../../include/fog.sh"

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

void main() {
	vec4 albedo = texture2DArray(s_albedo, vec3(v_texcoord.xy, v_data.x)) * v_color0 * u_colorOffset;
	if (albedo.a <= 0.0) discard;

	vec4 normal = texture2DArray(s_normal, vec3(v_texcoord.xy, v_data.x));
	vec4 specular = texture2DArray(s_specular, vec3(v_texcoord.xy, v_data.x)) * v_color0;
	vec4 emission = texture2DArray(s_emission, vec3(v_texcoord.xy, v_data.x)) * v_color0 * u_colorOffset;

	// ----
	vec3 norm = normalize(v_normal);
	//vec3 modelNormal = convertTangentNormal(v_normal, v_tangent, normal);
	// -----

	vec3 viewDir = normalize(u_camPos - v_worldPos);

	// Apply lights ----
	vec3 radianceOut = applyLight(gl_FragCoord, v_worldPos, norm, viewDir, specular.r, specularPower);
	radianceOut += emission;

	gl_FragData[0].rgb = albedo * radianceOut;
	gl_FragData[0].a = albedo.a; // COLOR
	// -------

	// Apply Fog ----
	gl_FragData[0] = applyFog(gl_FragData[0], v_worldPos, u_camPos);
	// -------

	gl_FragData[1].r = 1.F - recieve_decals; // DECALS

 	// GPU PICKING -----
	bool alpha = v_gpuPick.r == 0. && v_gpuPick.g == 0. && v_gpuPick.b == 0.;
	gl_FragData[2].rgba = vec4(v_gpuPick.rgb, alpha ? 0. : 1.);
	// -----------------
}
