$input v_color0, v_texcoord0, v_normal, v_tangent, v_bitangent, v_view

#include <bgfx_shader.sh>
#include <../../include/model_light.sh>

SAMPLER2D(s_texColor, 0);

uniform vec4 u_colorOffset;

void main() {
	vec4 texColor = texture2D(s_texColor, v_texcoord0.xy) * v_color0 * u_colorOffset;
	if (texColor.a <= 0.0) discard;

	gl_FragColor = calcLight(v_view, normalize(v_normal), texColor, v_texcoord0);
}
