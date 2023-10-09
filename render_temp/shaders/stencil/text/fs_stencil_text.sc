$input v_color0, v_texcoord

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main() {
	float a = texture2D(s_texColor, v_texcoord).a * v_color0.a;
	if(a <= 0.0) discard;

	gl_FragColor = vec4(v_color0.rgb, a);
}
