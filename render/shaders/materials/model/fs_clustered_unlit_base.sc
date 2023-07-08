
$input v_texcoord0, v_color0

#include <bgfx_shader.sh>
#include "../../include/defs.sh"

SAMPLER2D(s_albedo, SAMPLE_MAT_ALBEDO);

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

void main() {
	gl_FragColor = texture2D(s_albedo, v_texcoord0.xy) * v_color0 * u_colorOffset;
}
