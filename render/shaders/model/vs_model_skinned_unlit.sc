$input a_position, a_color0, a_texcoord0, a_indices, a_weight
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../include/model_psx.sh>
#include <../include/model_skinned.sh>


void main() {
	vec4 translated = mul(u_modelViewProj, boneTransform(a_indices, a_weight, a_position));
	gl_Position = psx_snap(translated, u_viewRect.zw / 2.);


	v_texcoord0 = a_texcoord0;;
	v_color0 = a_color0;
}
