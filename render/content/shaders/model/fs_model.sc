$input v_color0, v_texcoord0, v_translated0

#include <bgfx_shader.sh>
#include <../include/model_light.sh>

SAMPLER2D(s_texColor, 0);

void main() {
	vec4 texColor = texture2D(s_texColor, v_texcoord0.xy) * v_color0;
	if (texColor.a <= 0.0) discard;

	gl_FragColor = calculateLight(texColor, v_translated0, vec3(0, 0, 1));
}
