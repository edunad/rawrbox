$input v_texcoord0, v_color0, v_model_0, v_model_1, v_model_2, v_model_3, v_normal, v_tangent, v_worldPos

#include <bgfx_shader.sh>

#include "../../include/defs.sh"
#include "../../include/shaderlib.sh"
#include "../../include/utils.sh"

SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);
SAMPLER2DARRAY(s_normal, SAMPLE_MAT_NORMAL);
SAMPLER2DARRAY(s_specular, SAMPLE_MAT_SPECULAR);
SAMPLER2DARRAY(s_emission, SAMPLE_MAT_EMISSION);

SAMPLER2D(s_depth, SAMPLE_DEPTH);
SAMPLER2D(s_mask, SAMPLE_MASK);

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

uniform vec4 u_texMatData;
uniform vec4 u_decalSettings;

void main() {
#if BGFX_SHADER_LANGUAGE_HLSL && (BGFX_SHADER_LANGUAGE_HLSL < 400)
	vec2 screenPosition = gl_FragCoord.xy * u_viewTexel.xy + u_viewTexel.xy * vec2_splat(0.5);
#else
	vec2 screenPosition = gl_FragCoord.xy * u_viewTexel.xy;
#endif

	float rawDepth = texture2D(s_depth, screenPosition).r;
	float mask = texture2D(s_mask, screenPosition).r;
    if(mask == 1.0) discard;

	vec3 worldPosition = reconstructWorldPosition(screenPosition, rawDepth);
	mat4 model = mtxFromCols(v_model_0, v_model_1, v_model_2, v_model_3);

	vec4 objectPosition = mul(inverse(model), vec4(worldPosition, 1.0));
	if(any(greaterThan(abs(objectPosition.xyz), vec3_splat(0.5)))) {
        if(u_decalSettings.x == 0.0) discard;
        gl_FragColor = vec4(0.8, 0.0, 0.0, 0.25);
        return;
    }else if(u_decalSettings.x == 1.0) { // Show debug
        gl_FragColor = vec4(0.8, 0.8, 0.0, 1.0);
        return;
    }

	vec2 decalTexCoord = vec2(0.5 + objectPosition.y, objectPosition.x - 0.5);
	decalTexCoord = objectPosition.xy + 0.5;
    decalTexCoord.xy = 1.0 - decalTexCoord.xy; // Flip textures


    // Apply materials -----
    vec4 albedo = texture2DArray(s_albedo, vec3(decalTexCoord.xy, v_texcoord0.z));
    if(albedo.a <= 0.0) discard;
/*
	vec4 normal = texture2DArray(s_normal, vec3(v_texcoord0.xy, v_texcoord0.z));
	vec4 specular = texture2DArray(s_specular, vec3(v_texcoord0.xy, v_texcoord0.z)) * v_color0;
	vec4 emission = texture2DArray(s_emission, vec3(v_texcoord0.xy, v_texcoord0.z)) * v_color0 * u_colorOffset;

	// ----
	vec3 norm = normalize(v_normal);
    vec3 modelNormal = convertTangentNormal(v_normal, v_tangent, normal);
	// -----

    vec3 viewDir = normalize(u_camPos - v_worldPos);
    // Apply light ----

    // ----*/

    gl_FragColor = albedo * v_color0;
}
