#ifndef INCLUDED_MODEL_LIGHT
#define INCLUDED_MODEL_LIGHT

// Lighting
uniform vec4 u_lightsSetting;
uniform mat4 u_lightsPosition[12];
uniform mat4 u_lightsData[12];
// -----

// From https://github.com/SonarSystems/Modern-OpenGL-Tutorials/blob/master/%5BLIGHTING%5D/%5B14%5D%20Combining%20Directional%2C%20Point%20and%20Spot%20Lights/res/shaders/lighting.frag
vec3 calculatePointLight(vec3 pos, mat4 lightData, vec3 fragPos, vec3 viewDir, vec3 vNormal) {
	vec3 lightDir = normalize(pos - fragPos);

	// Diffuse shading
	float diff = max(dot(vNormal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, vNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.0); // material.shininess

	// Attenuation
	float distance = length(pos - fragPos);
	float light_constant = lightData[0][3];
	float light_linear = lightData[1][3];
	float light_quadratic = lightData[2][3];

	float attenuation = 2.0f; // 1.0f / (light_constant + light_linear * distance + light_quadratic * (distance * distance));

	// Combine results
	vec3 ambient = vec3(0, 0, 0);
	vec3 diffuse = vec3(lightData[0][0], lightData[0][1], lightData[0][2]) * diff;

	ambient *= attenuation;
	diffuse *= attenuation;

	return diffuse + ambient;
}

vec3 calculateSpotLight(vec3 pos, mat4 lightData, vec3 fragPos, vec3 viewDir, vec3 vNormal) {
	vec3 dir = vec3(lightData[1][0], lightData[1][1], lightData[1][2]);
	vec3 lightDir = normalize(pos - fragPos);

	// Diffuse shading
	float diff = max(dot(vNormal, lightDir), 0.0);

	// Specular shading
	vec3 reflectDir = reflect(-lightDir, vNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.0); // material.shininess

	// Attenuation
	float light_constant = lightData[0][3];
	float light_linear = lightData[1][3];
	float light_quadratic = lightData[2][3];

	float distance = length(pos - fragPos);
	float attenuation = 2.0f; // 1.0f / (light_constant + light_linear * distance + light_quadratic * (distance * distance));

	// Spotlight intensity
	float theta = dot(lightDir, normalize(-dir));
	float epsilon = lightData[2][0] - lightData[2][1];
	float intensity = clamp((theta - lightData[2][1]) / epsilon, 0.0, 1.0);

	// Combine results
	vec3 ambient = vec3(0, 0, 0);
	vec3 diffuse = vec3(lightData[0][0], lightData[0][1], lightData[0][2]) * diff;

	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;

	return diffuse + ambient;
}
#endif
