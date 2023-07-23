
#include <bgfx_compute.sh>
#include <bgfx_shader.sh>

#define READ_G_BUFFER

#include "../include/material.sh"
#include "../include/lights.sh"
#include "../include/utils.sh"

uniform vec4 u_lightIndexVec;
#define u_lightIndex uint(u_lightIndexVec.x)

uniform vec4 u_camPos;

void main() {
    vec2 texcoord = gl_FragCoord.xy / u_viewRect.zw;

    vec4 diffuseColor = texture2D(s_texDiffuse, texcoord);
    vec3 normal = unpackNormal(texture2D(s_texNormal, texcoord).xy);


    // get fragment position
    // rendering happens in view space
    vec4 screen = gl_FragCoord;
    screen.z = texture2D(s_texDepth, texcoord).x;

    vec3 fragPos = screen2Eye(screen).xyz;
    vec3 viewDir = normalize(u_camPos - fragPos);

    // Light
    vec3 radianceOut = vec3_splat(0.0);

    Light light = getLight(u_lightIndex);
    light.position = mul(u_view, vec4(light.position, 1.0)).xyz;

    vec3 lightDir = normalize(light.position - fragPos);
    float dist = distance(light.position, fragPos);
    float NdotL = dot(normal, lightDir);

    /// ---- POINT
    float attenuation = smoothAttenuation(dist, light.radius);
    float reflection = 0.0;

    if(attenuation > 0.0) {
       /* if (NdotL > 0.0 && reflection > 0.0) {
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), reflection);
            radianceOut += light.intensity * spec * specular; // Specular
        }*/

        radianceOut += light.intensity * attenuation; // Diffuse
    }

    gl_FragColor = vec4(radianceOut, 1.0);
}
