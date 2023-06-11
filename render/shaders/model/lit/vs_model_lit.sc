$input a_position, a_normal, a_tangent, a_bitangent, a_color0, a_texcoord0
$output v_color0, v_texcoord0, v_wPos, v_normal, v_tangent, v_bitangent

#include <bgfx_shader.sh>
#include <../../include/model_psx.sh>

uniform vec3 u_mesh_pos;
uniform vec4 u_data;

SAMPLER2D(s_heightMap, 1);

void main() {
	vec3 vPos = a_position.xyz;

    // displacement mode
    if(u_data.z != 0.) {
	    vPos.y += texture2DLod(s_heightMap, a_texcoord0, 0).x * u_data.z;
    }

	vec4 world = mul(u_model[0], vec4(vPos, 1.0) );
    vec4 translated = mul(u_viewProj, world);

	// vertex_snap mode
    if(u_data.y != 0.) {
        translated = psx_snap(translated, u_viewRect.zw / u_data.y);
    }
	// ----

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0;

	vec3 normal = a_normal * 2.0 - 1.0;
	vec3 tangent = a_tangent * 2.0 - 1.0;
	vec3 bitagent = a_bitangent * 2.0 - 1.0;
	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0) ).xyz;
	vec3 wtangent = mul(u_model[0], vec4(tangent.xyz, 0.0) ).xyz;
	vec3 wbitagent = mul(u_model[0], vec4(tangent.xyz, 0.0) ).xyz;

	v_normal = normalize(wnormal);
	v_tangent = normalize(wtangent);
	v_bitangent = normalize(wbitagent);

	v_wPos = world.xyz;
	gl_Position = translated;
}
