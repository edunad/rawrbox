$input a_position, a_color0, a_normal, a_texcoord0
$output v_normal, v_texcoord0, v_texcoord1, v_color0

#include <bgfx_shader.sh>
//#include <../include/model_transforms.sh>

void main() {
	/*vec3 pos = a_position.xyz;

	vec3 normal = a_normal * 2.0 - 1.0;
	vec3 tangent = a_tangent * 2.0 - 1.0;
	vec3 bitagent = a_bitangent * 2.0 - 1.0;

	v_normal = mul(u_model[0], vec4(normal.xyz, 0.0)).xyz;
	v_tangent = mul(u_model[0], vec4(tangent.xyz, 0.0)).xyz;
	v_bitangent = mul(u_model[0], vec4(bitagent.xyz, 0.0)).xyz;

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0.xy;
	v_texcoord1 = vec4(mul(u_model[0], vec4(pos, 1.0)).xyz, 1.0); // Pass through world space position

    gl_Position = mul(u_modelViewProj, vec4(pos, 1.0)); //applyPosTransforms(a_position, v_texcoord0);
	*/


    v_color0 = a_color0;

	// Calculate vertex position
	vec3 pos = a_position.xyz;
	gl_Position = mul(u_modelViewProj, vec4(pos, 1.0));

	// Calculate normal, unpack
	vec3 osNormal = a_normal.xyz * 2.0 - 1.0;

	// Transform normal into world space
	vec3 wsNormal = mul(u_model[0], vec4(osNormal, 0.0)).xyz;
	v_normal.xyz = normalize(wsNormal);

	v_texcoord0 = a_texcoord0;

	// Pass through world space position
	vec3 wsPos  = mul(u_model[0], vec4(pos, 1.0)).xyz;
	v_texcoord1 = vec4(wsPos, 1.0);
}
