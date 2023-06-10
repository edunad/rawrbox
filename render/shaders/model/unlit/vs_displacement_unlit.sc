$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../../include/model_psx.sh>

uniform vec3 u_mesh_pos;
uniform vec4 u_data;

SAMPLER2D(s_heightMap, 1);

void main() {
	vec3 vPos = a_position.xyz;
	vPos.y = texture2DLod(s_heightMap, a_texcoord0, 0).x * 16.0;

	gl_Position = mul(u_modelViewProj, vec4(vPos.xyz, 1.0));

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0;
}
