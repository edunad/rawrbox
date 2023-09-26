$input v_color0, v_texcoord

#include <bgfx_shader.sh>

SAMPLER2DARRAY(s_texColor, 0);

void main() {
	vec4 rgba = texture2DArray(s_texColor, vec3(v_texcoord.xy, v_texcoord.w)).xxxx;

	rgba.xyz = rgba.xyz * v_color0.xyz * rgba.w * v_color0.w;
	rgba.w   = rgba.w * v_color0.w * (1.0f - v_texcoord.z);
	if(rgba.w <= 0.085) discard;

	gl_FragColor = rgba;
}
