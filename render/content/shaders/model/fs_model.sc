$input v_color0, v_texcoord0, v_fragPos0, v_fragPos1, v_normal

#include <bgfx_shader.sh>
#include <../include/model_light.sh>

SAMPLER2D(s_texColor, 0);
uniform vec4 u_viewPos;

void main() {
	vec4 texColor = texture2D(s_texColor, v_texcoord0.xy) * v_color0;
	if (texColor.a <= 0.0) discard;
    vec3 viewDir = normalize( u_viewPos.xyz - v_fragPos0 );

	if(u_lightsSetting.x != 1.0) {
		vec3 ambient = vec3(0, 0, 0);

		// Light parsing
		int totalPoint = int(u_lightsSetting.y);
		int totalSpot = int(u_lightsSetting.z);
		int totalDir = int(u_lightsSetting.w);

		int totalLights = totalPoint + totalSpot + totalDir;
		for(int i = 0; i < totalLights; i++) {
			if(u_lightsData[i][3][3] != 1.0) continue;

			vec3 lightPos = mul(u_lightsPosition[i], vec4(v_fragPos0, 1.0)).xyz;
			if(i <= totalPoint) { // POINT
				ambient += calculatePointLight(lightPos, u_lightsData[i], v_fragPos0, viewDir, v_normal);
			}else if(i > totalPoint && i <= totalPoint + totalSpot) { // SPOT
				ambient += calculateSpotLight(lightPos, u_lightsData[i], v_fragPos0, viewDir, v_normal);
			}else if(i > totalPoint + totalSpot && i <= totalLights) { // DIR
				//ambient += calculateDirLight(lightPos, u_lightsData[i], v_fragPos0, viewDir, v_normal);
			}
		}

		gl_FragColor = vec4(ambient, 1.0) * texColor;
	} else {
		gl_FragColor = texColor; // Full bright
	}

}