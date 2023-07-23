
$input v_texcoord0, v_color0

#define VERTEX_DATA

#include <bgfx_shader.sh>
#include "../../include/defs.sh"
#include "../../include/utils.sh"
#include "../../include/material.sh"

SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);

uniform vec4 u_colorOffset;

void main() {
	vec4 albedo = texture2DArray(s_albedo, vec3(v_texcoord0.xy, v_texcoord0.z)) * v_color0 * u_colorOffset;
	if (albedo.a <= 0.0) discard;

    // pack G-Buffer
    gl_FragData[0] = albedo;
    gl_FragData[1] = vec4(0.0, 0.0, 0.0, 1.0);
    gl_FragData[2] = vec4(0.0, 0.0, 0.0, 1.0);
    gl_FragData[3].r = 1.F - recieve_decals; // DECALS
}
