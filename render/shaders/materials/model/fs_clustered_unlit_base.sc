
$input v_texcoord0, v_color0

#include <bgfx_shader.sh>
#include "../../include/defs.sh"
#include "../../include/model_transforms.sh"

SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

void main() {
	vec4 albedo = texture2DArray(s_albedo, vec3(v_texcoord0.xy, v_texcoord0.z)) * v_color0 * u_colorOffset;
	if (albedo.a <= 0.0) discard;

    gl_FragData[0] = albedo; // COLOR
	gl_FragData[1].r = 1.F - recieve_decals; // DECALS
}
