$input v_color0, v_texcoord0, v_wPos, v_normal, v_tangent, v_bitangent

#include <bgfx_shader.sh>
#include <../../include/model_light.sh>

SAMPLER2D(s_texColor, 0);
SAMPLER2D(s_texSpecularColor, 1);

uniform vec4 u_cameraPos;
uniform vec4 u_colorOffset;
uniform vec2 u_texSpecularShininess;

void main() {
	vec4 specularColor = texture2D(s_texSpecularColor, v_texcoord0.xy) * v_color0 * u_colorOffset;
	vec4 texColor = texture2D(s_texColor, v_texcoord0.xy) * v_color0 * u_colorOffset;
	if (texColor.a <= 0.0) discard;

    vec3 viewDir = normalize(u_cameraPos.xyz - v_wPos);
	//mat3 tbn = mtxFromCols(v_tangent, v_bitangent, v_normal);

	if(u_lightsSetting.x != 1.0) {
		vec3 ambient = vec3(0, 0, 0);

		// Light parsing
		int totalLights = int(u_lightsSetting.y);
		for(int i = 0; i < totalLights; i++) {
			if(u_lightsData[i][3][3] != 1.0) continue; // Is it on?
			vec3 lightPos = vec3(u_lightsPosition[i][0], u_lightsPosition[i][1], u_lightsPosition[i][2]);

			if(u_lightsData[i][3][0] == 1.0) { // POINT
				ambient += calculatePointLight(lightPos, u_lightsData[i], v_wPos, viewDir, v_normal, texColor, specularColor, u_texSpecularShininess);
			}else if(u_lightsData[i][3][0] == 2.0) { // SPOT
				ambient += calculateSpotLight(lightPos, u_lightsData[i], v_wPos, viewDir, v_normal, texColor, specularColor, u_texSpecularShininess);
			}else if(u_lightsData[i][3][0] == 3.0) { // DIR
				ambient += calculateDirectionalLight(lightPos, u_lightsData[i], v_wPos, viewDir, v_normal, texColor, specularColor, u_texSpecularShininess);
			}
		}

		gl_FragColor = vec4(ambient, texColor.a);
	} else {
		gl_FragColor = texColor; // Full bright
	}
}
