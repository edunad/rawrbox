$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main() {
	float thickness = 0.5;
	float softness = 0.1;

	vec4 outline_color = vec4(1., 1., 1., 1.);
	float outline_thickness = 0.0;
	float outline_softness = 0.1;

    float a = texture2D(s_texColor, v_texcoord0.xy).r;
	float outline = smoothstep(outline_thickness - outline_softness, outline_thickness + outline_softness, a);
	a = smoothstep(1.0 - thickness - softness, 1.0 - thickness + softness, a);

	gl_FragColor = vec4(mix(outline_color.rgb, v_color0.rgb, outline), a);
}
