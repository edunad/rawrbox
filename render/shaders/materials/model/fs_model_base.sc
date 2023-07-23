
$input v_normal, v_tangent, v_texcoord0, v_color0

#define VERTEX_DATA

#include <bgfx_shader.sh>
#include "../../include/defs.sh"
#include "../../include/utils.sh"
#include "../../include/material.sh"

SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);
SAMPLER2DARRAY(s_normal, SAMPLE_MAT_NORMAL);
SAMPLER2DARRAY(s_specular, SAMPLE_MAT_SPECULAR);
SAMPLER2DARRAY(s_emission, SAMPLE_MAT_EMISSION);

uniform vec4 u_colorOffset;

void main() {
	vec4 albedo = texture2DArray(s_albedo, vec3(v_texcoord0.xy, v_texcoord0.z)) * v_color0 * u_colorOffset;
	if (albedo.a <= 0.0) discard;

	vec4 normal = texture2DArray(s_normal, vec3(v_texcoord0.xy, v_texcoord0.z));
	vec4 specular = texture2DArray(s_specular, vec3(v_texcoord0.xy, v_texcoord0.z));
	vec4 emission = texture2DArray(s_emission, vec3(v_texcoord0.xy, v_texcoord0.z)) * v_color0 * u_colorOffset;

    vec3 N = convertTangentNormal(v_normal, v_tangent, normal.rgb);

    // pack G-Buffer
    gl_FragData[0] = albedo;
    gl_FragData[1] = vec4(packNormal(N), 0.0, 0.0);
    gl_FragData[2] = vec4(emission.rgb, specular.r);
    gl_FragData[3].r = 1.F - recieve_decals; // DECALS
}
