$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor,  0);

void main() {
	vec4 texColor = texture2D(s_texColor, v_texcoord0) * v_color0;
	if (texColor.a <= 0.0) discard;

	gl_FragColor = texColor;
}
