$input v_texcoord, v_color0, v_model_0, v_model_1, v_model_2, v_model_3, v_normal, v_worldPos

#define READ_LIGHT_INDICES
#define READ_LIGHT_GRID

#define READ_MATERIAL
#define READ_DEPTH
#define READ_MASK

#include <bgfx_shader.sh>

#include "../../include/clusters.sh"
#include "../../include/lights.sh"
#include "../../include/shaderlib.sh"
#include "../../include/utils.sh"
#include "../../include/material.sh"
#include "../../include/fog.sh"

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

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
	vec4 albedo = texture2DArray(s_albedo, vec3(decalTexCoord.xy, v_texcoord.z));
	if(albedo.a <= 0.0) discard;

	// ----
	vec3 norm = normalize(v_normal);
	// -----

	// Apply light ----
	vec3 radianceOut = applyLight(gl_FragCoord, v_worldPos, norm, vec3(0.0, 0.0, 0.0), 0.0, 0.0);

	gl_FragColor.rgb = albedo * radianceOut * v_color0;
	gl_FragColor.a = albedo.a; // COLOR
	// ----

	// Apply Fog ----
	gl_FragColor = applyFog(gl_FragColor, v_worldPos, u_camPos);
	// -------
}
