$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main() {
	float thickness = 0.85;
	float softness = 0.5;

    vec2 texCol = texture2D(s_texColor, v_texcoord0.xy).rg;
	//texCol.r = smoothstep((1.0 - thickness) - softness, (1.0 - thickness) + softness, texCol.r);

	gl_FragColor = vec4(v_color0.rgb, texCol.r);
}
