
$input v_normal, v_tangent, v_bitangent, v_texcoord0, v_color0

#include <bgfx_shader.sh>
#include <../include/shaderlib.sh>

SAMPLER2D(s_albedo, 0);
SAMPLER2D(s_normal, 1);
SAMPLER2D(s_specular, 2);

uniform vec4 u_colorOffset;

void main() {
	vec4 albedo = texture2D(s_albedo, v_texcoord0) * v_color0 * u_colorOffset;

	float specular = texture2D(s_specular, v_texcoord0).r;
	albedo.a = specular;

	vec3 normal;
	normal.xy = texture2D(s_normal, v_texcoord0).xy * 2.0 - 1.0;
	normal.z  = sqrt(1.0 - dot(normal.xy, normal.xy) );

	mat3 tbn = mat3(
				normalize(v_tangent),
				normalize(v_bitangent),
				normalize(v_normal)
				);


	// get vertex normal
	normal = normalize(mul(tbn, normal));
	vec3 wnormal = normalize(mul(u_invView, vec4(normal, 0.0) ).xyz);

	// Write the data on the G-BUFFER
	gl_FragData[0] = albedo;
	gl_FragData[1] = vec4(encodeNormalUint(wnormal), 1.0);
}
