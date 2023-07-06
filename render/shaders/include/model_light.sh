/*#ifndef INCLUDED_MODEL_LIGHT
#define INCLUDED_MODEL_LIGHT

#define MAX_LIGHTS 8

SAMPLER2D(s_texSpecularColor, 2);
SAMPLER2D(s_texEmissionColor, 3);
SAMPLER2D(s_texOpacityColor, 4);

// Lighting
uniform vec4 u_lightsSetting;
uniform vec3 u_lightsPosition[MAX_LIGHTS];
uniform mat4 u_lightsData[MAX_LIGHTS];

uniform vec4 u_specularColor;
uniform vec4 u_emissionColor;
// -----

uniform vec2 u_texMatData; // x = shininess, y = emission strength

vec2 blinn(vec3 _lightDir, vec3 _normal, vec3 _viewDir) {
	float ndotl = dot(_normal, _lightDir);
	vec3 reflected = 2.0 * ndotl * _normal - _lightDir;
	float rdotv = dot(reflected, _viewDir);
	return vec2(ndotl, rdotv);
}

vec4 lit(float _ndotl, float _rdotv, float _m) {
	float diff = max(0.0, _ndotl);
	float spec = step(0.0, _ndotl) * pow(max(0.0, _rdotv), _m);
	return vec4(1.0, diff, spec, 1.0);
}

vec3 calculatePointLight(int index, vec3 view, vec3 normal, vec4 texColor, vec4 specularColor) {
	mat4 lData = u_lightsData[index];

	// Calculate light positions
	vec3 toLight = mul(u_view, vec4(u_lightsPosition[index].xyz, 1.0)).xyz - view;
	vec3 lightDir = normalize(toLight);

	vec2 bln = blinn(lightDir, normal, -normalize(view));
	vec4 lc = lit(bln.x, bln.y, u_texMatData.x);
	// ------

	// Diffuse shading
	vec3 diffuse = vec3(lData[0][0], lData[0][1], lData[0][2]) * lc.y;

	// Specular shading
	vec3 specular = vec3(lData[1][0], lData[1][1], lData[1][2]) * lc.z;

	// Power
	float distance = length(toLight);
	float light_constant = lData[0][3];
	float light_linear = lData[1][3];
	float light_quadratic = lData[2][3];

	float attenuation = 1.0 / (light_constant + light_linear * distance + light_quadratic * distance * distance);

	vec3 dif = texColor.rgb * (diffuse * attenuation);
	vec3 spec = specularColor.r * (specular * attenuation);

	return dif + spec;
}

vec3 calculateSpotLight(int index, vec3 view, vec3 normal, vec4 texColor, vec4 specularColor) {
	mat4 lData = u_lightsData[index];

	// Calculate light positions
	vec3 toLight = mul(u_view, vec4(u_lightsPosition[index].xyz, 1.0)).xyz - view;
	vec3 lightDir = normalize(toLight);

	vec2 bln = blinn(lightDir, normal, -normalize(view));
	vec4 lc = lit(bln.x, bln.y, u_texMatData.x);
	// ------

	// Diffuse shading
	vec3 diffuse = vec3(lData[0][0], lData[0][1], lData[0][2]) * lc.y;

	// Specular shading
	vec3 specular = vec3(lData[1][0], lData[1][1], lData[1][2]) * lc.z;

	// Spotlight intensity
	float theta = dot(vec3(lData[2][0], lData[2][1], lData[2][2]), -lightDir);
	float intensity = clamp((theta - lData[3][1]) / (lData[3][2] - lData[3][1]), 0.0f, 1.0f);

	// Spotlight power
	float distance = length(toLight);
	float light_constant = lData[0][3];
	float light_linear = lData[1][3];
	float light_quadratic = lData[2][3];

	float attenuation = 1.0 / (light_constant + light_linear * distance + light_quadratic * distance * distance);

	vec3 dif = texColor.rgb * (diffuse.rgb  * attenuation);
	vec3 spec = specularColor.r * (specular.rgb * intensity * attenuation);

	return dif + spec;
}

vec3 calculateDirectionalLight(int index, vec3 view, vec3 normal, vec4 texColor, vec4 specularColor) {
	mat4 lData = u_lightsData[index];

	// Calculate light positions
	vec3 toLight = mul(u_view, vec4(u_lightsPosition[index].xyz, 1.0)).xyz - view;
	vec3 lightDir = normalize(toLight);

	vec2 bln = blinn(lightDir, normal, -normalize(view));
	vec4 lc = lit(bln.x, bln.y, u_texMatData.x);
	// ------

	float theta = dot(vec3(lData[2][0], lData[2][1], lData[2][2]), -lightDir);

	// Diffuse shading
	vec3 diffuse = vec3(lData[0][0], lData[0][1], lData[0][2]) * lc.y;

	// Specular shading
	vec3 specular = vec3(lData[1][0], lData[1][1], lData[1][2]) * lc.z;

	vec3 dif = texColor.rgb * (diffuse.rgb);
	vec3 spec = specularColor.r * (specular.rgb * theta);

	return dif + spec;
}


vec4 calcLight(vec3 v_view, vec3 normal, vec4 texColor, vec2 v_texcoord0) {
	vec4 specularColor = texture2D(s_texSpecularColor, v_texcoord0.xy) * u_specularColor;
	vec4 emissionColor = texture2D(s_texEmissionColor, v_texcoord0.xy) * u_emissionColor;
	vec4 opacityColor = texture2D(s_texOpacityColor, v_texcoord0.xy);

	if(u_lightsSetting.x == 0.0){
		vec3 ambient = vec3_splat(0.0);

		// Light parsing
		int totalLights = int(u_lightsSetting.y);
		for(int i = 0; i < totalLights; i++) {
			if(u_lightsData[i][3][3] != 1.0) continue; // Is it on?

			if(u_lightsData[i][3][0] == 1.0) { // POINT
				ambient += calculatePointLight(i, v_view, normal, texColor, specularColor);
			}else if(u_lightsData[i][3][0] == 2.0) { // SPOT
				ambient += calculateSpotLight(i, v_view, normal, texColor, specularColor);
			}else if(u_lightsData[i][3][0] == 3.0) { // DIR
				ambient += calculateDirectionalLight(i, v_view, normal, texColor, specularColor);
			}
		}

		return vec4(ambient + (emissionColor.rgb * u_texMatData.y), opacityColor.a);
	} else {
		return texColor; // Fullbright
	}
}

#endif
*/
