$input v_color0, v_texcoord0, v_texcoord1, v_normal, v_tangent, v_bitangent

#include <bgfx_shader.sh>
#include <../include/normal_encoding.sh>

vec3 toLinear(vec3 _rgb) {
	return pow(abs(_rgb), vec3_splat(2.2) );
}

vec3 toGamma(vec3 _rgb) {
	return pow(abs(_rgb), vec3_splat(1.0/2.2) );
}

// http://www.thetenthplanet.de/archives/1180
// "followup: normal mapping without precomputed tangents"
mat3 cotangentFrame(vec3 N, vec3 p, vec2 uv) {
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx(p);
	vec3 dp2 = dFdy(p);
	vec2 duv1 = dFdx(uv);
	vec2 duv2 = dFdy(uv);

	// solve the linear system
	vec3 dp2perp = cross(dp2, N);
	vec3 dp1perp = cross(N, dp1);
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame
	float invMax = inversesqrt(max(dot(T,T), dot(B,B)));
	return mat3(T*invMax, B*invMax, N);
}

// Texture samples ---
SAMPLER2D(s_albedo, 0);
SAMPLER2D(s_normal, 1);
SAMPLER2D(s_specular, 2);
SAMPLER2D(s_emission, 3);
SAMPLER2D(s_opacity, 4);
// ----

void main() {
	vec3 albedo = toLinear(texture2D(s_albedo, v_texcoord0).xyz);

	// get vertex normal
	vec3 normal = normalize(v_normal);

	// get normal map normal, unpack, and calculate z
	vec3 normalMap;
	normalMap.xy = texture2D(s_normal, v_texcoord0).xy;
	normalMap.xy = normalMap.xy * 2.0 - 1.0;
	normalMap.z = sqrt(1.0 - dot(normalMap.xy, normalMap.xy));

	// perturb geometry normal by normal map
	vec3 pos = v_texcoord1.xyz; // contains world space pos
	mat3 TBN = cotangentFrame(normal, pos, v_texcoord0);
	vec3 bumpedNormal = normalize(instMul(TBN, normalMap));

	// need some proxy for roughness value w/o roughness texture
	// assume horizontal (blue) normal map is smooth, and then
	// modulate with albedo for some higher frequency detail
	float roughness = normalMap.z * mix(0.9, 1.0, albedo.y);
	roughness = roughness * 0.6 + 0.2;

	vec3 bufferNormal = NormalEncode(bumpedNormal);
	gl_FragData[0] = vec4(toGamma(albedo), 1.0);
	gl_FragData[1] = vec4(bufferNormal, roughness);
}

/*
SAMPLER2D(s_texColor, 0);

uniform vec4 u_colorOffset;

void main() {
	vec4 texColor = texture2D(s_texColor, v_texcoord0) * v_color0 * u_colorOffset;
	if (texColor.a <= 0.0) discard;

	gl_FragColor = calcLight(v_view, normalize(v_normal), texColor, v_texcoord0);
}
*/
