$input v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include "../../include/defs.sh"
#include "../../include/shaderlib.sh"

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

uniform mat4 u_decalMatrix;

SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);
SAMPLER2D(s_depth, SAMPLE_DEPTH);

vec3 reconstructWorldPosition(vec2 texCoord, float rawDepth) {
	vec3 clip = vec3(texCoord * 2.0 - 1.0, toClipSpaceDepth(rawDepth));
	#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
		clip.y = -clip.y;
	#endif
	vec4 wpos = mul(u_invViewProj, vec4(clip, 1.0));
	return wpos.xyz / wpos.w;
}

void main() {
	vec2 screenPosition = gl_FragCoord.xy * u_viewTexel.xy;
	float rawDepth = texture2D(s_depth, screenPosition).r;
	vec3 worldPosition = reconstructWorldPosition(screenPosition, rawDepth);

	vec4 objectPosition = mul(u_decalMatrix, vec4(worldPosition, 1.0));
	if(any(greaterThan(abs(objectPosition.xyz), vec3_splat(0.5)))) discard;

	vec2 decalTexCoord = vec2(0.5 - objectPosition.y, objectPosition.x + 0.5);
	decalTexCoord = objectPosition.xy + 0.5;

    vec4 albedo = texture2DArray(s_albedo, vec3(decalTexCoord.xy, 0)) * v_color0 * u_colorOffset;
	if (albedo.a <= 0.0) discard;

    gl_FragColor = albedo;
}
