$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../../include/model_psx.sh>

uniform vec3 u_mesh_pos;
uniform vec2 u_billboard;

void main() {
    if(u_billboard.x == 0.){
        vec4 translated = mul(u_modelViewProj, vec4(a_position, 1.0));
        gl_Position = psx_snap(translated, u_viewRect.zw / 2.);
    } else {
        vec3 right = vec3(u_invView[0][0], u_invView[1][0], u_invView[2][0]);
        vec3 up = vec3(u_invView[0][1], u_invView[1][1], u_invView[2][1]);

        vec3 pos = u_mesh_pos.xyz + (right * (a_position.x - u_mesh_pos.x)) + (up * (a_position.y - u_mesh_pos.y));
        gl_Position = psx_snap(mul(u_modelViewProj, vec4(pos, 1)), u_viewRect.zw / 2.);
    }

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0;
}
