$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);
void main() {
	vec4 rgba = texture2D(s_texColor, v_texcoord0.xy).xxxx;

	rgba.xyz = rgba.xyz * v_color0.xyz * rgba.w * v_color0.w;
	rgba.w   = rgba.w * v_color0.w * (1.0f - v_texcoord0.z);
	if(rgba.w <= 0.085) discard;

	gl_FragColor = rgba;
}
