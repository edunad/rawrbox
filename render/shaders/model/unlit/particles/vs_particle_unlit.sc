$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../../../include/model_psx.sh>

uniform vec3 u_mesh_pos;
uniform vec2 u_billboard;

void main() {
    vec4 translated = mul(u_modelViewProj, vec4(a_position, 1.0));
    gl_Position = psx_snap(translated, u_viewRect.zw / 2.);

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0;
}

