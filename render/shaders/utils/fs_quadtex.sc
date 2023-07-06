$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_sample, 0);

void main() {
    vec4 texColor = texture2D(s_sample, v_texcoord0.xy);
	if (texColor.a <= 0.0) discard;

	gl_FragColor = texColor;
}
