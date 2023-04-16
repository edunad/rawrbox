#ifndef INCLUDED_MODEL_GLSL
#define INCLUDED_MODEL_GLSL

// Lighting
uniform vec4 u_lightsSetting;
uniform mat4 u_lightsData[8];

vec4 calculateLight(vec4 mdl_color, vec4 translated, vec3 vNormal) {
	if (u_lightsSetting.x != 1.0) {
		vec4 color = vec4(0, 0, 0, 1); // AMBIENT COLOR;

		// Lighting
		vec3 vertToLight;

		float dotToLight;
		float distToLight;
		float power;

		int count = int(u_lightsSetting.y);
		for (int i = 0; i < count; i++) {
			if (u_lightsData[i][2][3] >= 1.0) {
				vertToLight = vec3(u_lightsData[i][0][0], u_lightsData[i][0][1], u_lightsData[i][0][2]) - translated.xyz;
				distToLight = length(vertToLight);

				power = (distToLight - u_lightsData[i][2][0]) / (u_lightsData[i][2][1] - u_lightsData[i][2][0]);
				power = clamp(power, 0., 1.);
				power = 1. - power;
				power *= power;

				// dotToLight = clamp(dot(normalize(vertToLight), vNormal), 0.f, 1.f);
				color += mdl_color * vec4(u_lightsData[i][1][0], u_lightsData[i][1][1], u_lightsData[i][1][2], u_lightsData[i][1][3]) * u_lightsData[i][2][2] * power;
			}
		}

		return color;
	} else {
		return mdl_color;
	}
}
#endif
