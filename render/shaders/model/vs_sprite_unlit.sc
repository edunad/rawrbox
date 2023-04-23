$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../include/model_psx.sh>

uniform vec3 u_sprite_pos;

// http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
void main() {
    vec3 up = vec3(u_invView[0][1], u_invView[1][1], u_invView[2][1]);
	vec3 right = vec3(u_invView[0][0], u_invView[1][0], u_invView[2][0]);
	vec3 pos = (right * a_position.x) + (up * a_position.y);

    gl_Position = mul(u_modelViewProj, vec4(pos + u_sprite_pos.xyz, 1));

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0;
}
