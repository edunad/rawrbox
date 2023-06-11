$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../../include/model_psx.sh>

uniform vec3 u_mesh_pos;
uniform vec4 u_data;

SAMPLER2D(s_heightMap, 1);

void main() {
    vec4 finalPos;
	vec3 vPos = a_position.xyz;

    // displacement mode
    if(u_data.z != 0.) {
	    vPos.y += texture2DLod(s_heightMap, a_texcoord0, 0).x * u_data.z;
    }

    // Billboard mode
    if(u_data.x == 0.){
        finalPos = mul(u_modelViewProj, vec4(vPos, 1.0));
    } else {
        vec3 right = vec3(u_invView[0][0], u_invView[1][0], u_invView[2][0]);
        vec3 up = vec3(u_invView[0][1], u_invView[1][1], u_invView[2][1]);

        vec3 pos = u_mesh_pos.xyz + (right * (vPos.x - u_mesh_pos.x)) + (up * (vPos.y - u_mesh_pos.y));
        finalPos = mul(u_modelViewProj, vec4(pos, 1));
    }

	// vertex_snap mode
    if(u_data.y != 0.) {
        finalPos = psx_snap(finalPos, u_viewRect.zw / u_data.y);
    }
	// ----

    gl_Position = finalPos;
    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0;
}
