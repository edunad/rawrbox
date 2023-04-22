#ifndef INCLUDED_MODEL_LIGHT
#define INCLUDED_MODEL_LIGHT

#define MAX_LIGHTS 8

// Lighting
uniform vec4 u_lightsSetting;
uniform vec4 u_lightsPosition[MAX_LIGHTS];
uniform mat4 u_lightsData[MAX_LIGHTS];
// -----
vec3 calculatePointLight(vec3 pos, mat4 lightData, vec3 worldPos, vec3 viewDir, vec3 vNormal, vec4 texColor, vec4 specularColor) {
	// Ambient
	vec3 ambient = vec3(0.1, 0.1, 0.1);
	vec3 lightDirection = normalize(pos - worldPos);

	// Diffuse shading
	vec3 diffuse = vec3(lightData[0][0], lightData[0][1], lightData[0][2]) * max(dot(vNormal, lightDirection), 0.0);

	// Specular shading
	float shininess = 10.0; // material.shininess
	vec3 reflectDir = reflect(-lightDirection, vNormal);
	vec3 specular = vec3(lightData[1][0], lightData[1][1], lightData[1][2]) * pow(max(dot(reflectDir, viewDir), 0.0), shininess);

	// Spotlight power
	float distance = length(pos - worldPos);
	float light_constant = lightData[0][3];
	float light_linear = lightData[1][3];
	float light_quadratic = lightData[2][3];
	float attenuation = 1.0 / (light_constant + light_linear * distance + light_quadratic * distance * distance);

	vec3 dif = texColor.rgb * (diffuse.rgb * attenuation + ambient);
	vec3 spec = specularColor.r * (specular.rgb * attenuation);

	return dif + spec;
}

vec3 calculateSpotLight(vec3 pos, mat4 lightData, vec3 worldPos, vec3 viewDir, vec3 vNormal, vec4 texColor, vec4 specularColor) {
	// Ambient
	vec3 ambient = vec3(0.1, 0.1, 0.1);
	vec3 lightDirection = normalize(pos - worldPos);

	// Diffuse shading
	vec3 diffuse = vec3(lightData[0][0], lightData[0][1], lightData[0][2]) * max(dot(vNormal, lightDirection), 0.0);

	// Specular shading
	float shininess = 20.0; // material.shininess
	vec3 reflectDir = reflect(-lightDirection, vNormal);
	vec3 specular = vec3(lightData[1][0], lightData[1][1], lightData[1][2]) * pow(max(dot(reflectDir, viewDir), 0.0), shininess);

	// Spotlight intensity
	float theta = dot(vec3(lightData[2][0], lightData[2][1], lightData[2][2]), -lightDirection);
	float intensity = clamp((theta - lightData[3][1]) / (lightData[3][2] - lightData[3][1]), 0.0f, 1.0f);

	// Spotlight power
	float distance = length(pos - worldPos);
	float light_constant = lightData[0][3];
	float light_linear = lightData[1][3];
	float light_quadratic = lightData[2][3];

	float attenuation = 1.0 / (light_constant + light_linear * distance + light_quadratic * distance * distance);

	vec3 dif = texColor.rgb * (diffuse.rgb * intensity * attenuation + ambient);
	vec3 spec = specularColor.r * (specular.rgb * intensity * attenuation);

	return dif + spec;
}

vec3 calculateDirectionalLight(vec3 pos, mat4 lightData, vec3 worldPos, vec3 viewDir, vec3 vNormal, vec4 texColor, vec4 specularColor) {

	// Ambient
	vec3 ambient = vec3(0.1, 0.1, 0.1);

	// Diffuse shading
	vec3 lightDirection = normalize(vec3(lightData[2][0], lightData[2][1], lightData[2][2]));
	vec3 diffuse = vec3(lightData[0][0], lightData[0][1], lightData[0][2]) * max(dot(vNormal, lightDirection), 0.0f);

	// specular lighting
	float shininess = 20.0; // material.shininess
	vec3 reflectionDirection = reflect(-lightDirection, vNormal);
	vec3 specular = vec3(lightData[1][0], lightData[1][1], lightData[1][2]) * pow(max(dot(reflectionDirection, viewDir), 0.0f), shininess);

	vec3 dif = texColor.rgb * (diffuse.rgb + ambient);
	vec3 spec = specularColor.r * specular.rgb;

	return dif + spec;
}

#endif
