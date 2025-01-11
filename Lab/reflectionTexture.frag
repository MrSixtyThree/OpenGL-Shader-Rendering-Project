#version 330 core

// interpolated values from vertex shaders
in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

// light properties
struct Light {
    vec3 pos;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
    vec3 att;  // constant, linear, quadratic
};

// material properties
struct Material {
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float shininess;
};

// uniform input data
uniform vec3 uViewpoint;
uniform Light uLight;
uniform Material uMaterial;
uniform sampler2D uTextureSampler;   // Texture
uniform samplerCube uEnvironmentMap; // Reflection map
uniform float uAlpha;                // Reflection blending factor

// output data
out vec3 fColor;

void main() {
    // fragment normal
    vec3 n = normalize(vNormal);

    // vector toward the viewer
    vec3 v = normalize(uViewpoint - vPosition);

    // vector towards the light
    vec3 l = normalize(uLight.pos - vPosition);

    // halfway vector
    vec3 h = normalize(l + v);

    // calculate ambient, diffuse and specular intensities
    vec3 Ia = uLight.La * uMaterial.Ka;
    vec3 Id = vec3(0.0f);
    vec3 Is = vec3(0.0f);
    float dotLN = max(dot(l, n), 0.0f);

    if (dotLN > 0.0f) {
        // attenuation
        float dist = length(uLight.pos - vPosition);
        float attenuation = 1.0f / (uLight.att.x + dist * uLight.att.y + dist * dist * uLight.att.z);

        Id = uLight.Ld * uMaterial.Kd * dotLN * attenuation;
        Is = uLight.Ls * uMaterial.Ks * pow(max(dot(n, h), 0.0f), uMaterial.shininess) * attenuation;
    }

    // sample the texture and modulate with lighting
    vec3 texColor = texture(uTextureSampler, vTexCoord).rgb;
    vec3 litTexColor = texColor * (Ia + Id + Is);

    // calculate reflection
    vec3 reflectEnvMap = reflect(-v, n);
    vec3 reflection = texture(uEnvironmentMap, reflectEnvMap).rgb;

    // combine texture and reflection based on uAlpha
    fColor = mix(litTexColor, reflection, uAlpha);
}
