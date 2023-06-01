$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main() {
	float a = texture2D(s_texColor, v_texcoord0.xy).a;
	if(a <= 0.0) discard;

	gl_FragColor = vec4(v_color0.rgb, a);
}
