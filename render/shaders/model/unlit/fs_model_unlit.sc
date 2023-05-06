$input v_color0, v_texcoord0

#include <bgfx_shader.sh>
SAMPLER2D(s_texColor, 0);

uniform vec4 u_cameraPos;
uniform vec4 u_colorOffset;

void main() {
	vec4 texColor = texture2D(s_texColor, v_texcoord0.xy) * v_color0 * u_colorOffset;
	if (texColor.a <= 0.01) discard;

	gl_FragColor = texColor; // Full bright
}
