#ifndef INCLUDED_MODEL_LIGHT
#define INCLUDED_MODEL_LIGHT

#define MAX_LIGHTS 8

// Lighting
uniform vec4 u_lightsSetting;
uniform mat4 u_lightsPosition[MAX_LIGHTS];
uniform mat4 u_lightsData[MAX_LIGHTS];
// -----

// From https://github.com/SonarSystems/Modern-OpenGL-Tutorials/blob/master/%5BLIGHTING%5D/%5B14%5D%20Combining%20Directional%2C%20Point%20and%20Spot%20Lights/res/shaders/lighting.frag
vec3 calculatePointLight(vec3 pos, mat4 lightData, vec3 fragPos, vec3 viewDir, vec3 vNormal, vec4 texColor, vec4 specularColor) {
	vec3 lightDir = normalize(pos - fragPos);

	// Diffuse shading
	float diff = max(dot(vNormal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, vNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0); // material.shininess

	// Attenuation
	float distance = length(pos - fragPos);
	float light_constant = lightData[0][3];
	float light_linear = lightData[1][3];
	float light_quadratic = lightData[2][3];

	float attenuation = 1.0f / (light_constant + light_linear * distance + light_quadratic * (distance * distance));

	// Combine results
	vec3 ambient = vec3(1, 1, 1);
	vec3 diffuse = vec3(lightData[0][0], lightData[0][1], lightData[0][2]) * diff * texColor.rgb;
	vec3 specular = vec3(lightData[1][0], lightData[1][1], lightData[1][2]) * spec * specularColor.rgb;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return ambient + diffuse + specular;
}

vec3 calculateSpotLight(vec3 pos, mat4 lightData, vec3 fragPos, vec3 viewDir, vec3 vNormal, vec4 texColor, vec4 specularColor) {
	vec3 dir = vec3(lightData[2][0], lightData[2][1], lightData[2][2]);
	vec3 lightDir = normalize(pos - fragPos);

	// Diffuse shading
	float diff = max(dot(vNormal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, vNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0); // material.shininess

	// Attenuation
	float light_constant = lightData[0][3];
	float light_linear = lightData[1][3];
	float light_quadratic = lightData[2][3];

	float distance = length(pos - fragPos);
	float attenuation = 1.0f / (light_constant + light_linear * distance + light_quadratic * (distance * distance));

	// Spotlight intensity
	float theta = dot(lightDir, normalize(-dir));
	float epsilon = lightData[3][0] - lightData[3][1];
	float intensity = clamp((theta - lightData[3][1]) / epsilon, 0.0, 1.0);

	// Combine results
	vec3 ambient = vec3(1, 1, 1);
	vec3 diffuse = vec3(lightData[0][0], lightData[0][1], lightData[0][2]) * diff * texColor.rgb;
	vec3 specular = vec3(lightData[1][0], lightData[1][1], lightData[1][2]) * spec * specularColor.rgb;

	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	return ambient + diffuse + specular;
}

vec3 calculateDirectionalLight(vec3 pos, mat4 lightData, vec3 fragPos, vec3 viewDir, vec3 vNormal, vec4 texColor, vec4 specularColor) {
	vec3 dir = vec3(lightData[2][0], lightData[2][1], lightData[2][2]);
	vec3 lightDir = normalize(-pos);

	// Diffuse shading
	float diff = max(dot(vNormal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, vNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0); // material.shininess

	// Combine results
	vec3 ambient = vec3(1, 1, 1);
	vec3 diffuse = vec3(lightData[0][0], lightData[0][1], lightData[0][2]) * diff * texColor.rgb;
	vec3 specular = vec3(lightData[1][0], lightData[1][1], lightData[1][2]) * spec * specularColor.rgb;

	return ambient + diffuse + specular;
}
#endif
