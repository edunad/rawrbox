$input v_texcoord0

#include <bgfx_shader.sh>
#include <../../include/shaderlib.sh>

SAMPLER2D(s_color, 0);
SAMPLER2D(s_light, 1);

void main() {
	vec4 albedo  = toLinear(texture2D(s_color,  v_texcoord0));
	vec4 light   = toLinear(texture2D(s_light,  v_texcoord0));

	gl_FragColor = toGamma(albedo * light);
}
