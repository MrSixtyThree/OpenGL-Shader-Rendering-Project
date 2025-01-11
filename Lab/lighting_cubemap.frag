#version 330 core

// interpolated values from the vertex shaders
in vec3 vPosition;
in vec3 vNormal;

// light properties
struct Light
{
	vec3 dir;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
};

// material properties
struct Material
{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float shininess;
};

// uniform input data
uniform vec3 uViewpoint;
uniform Light uLight;
uniform Material uMaterial;
uniform samplerCube uEnvironmentMap;

// output data
out vec3 fColor;

void main()
{
	// fragment normal
    vec3 n = normalize(vNormal);

	// vector toward the viewer
	vec3 v = normalize(uViewpoint - vPosition);

	// vector towards the light
    vec3 l = normalize(-uLight.dir);

	// halfway vector
	vec3 h = normalize(l + v);

	// calculate ambient, diffuse and specular intensities
	vec3 Ia = uLight.La * uMaterial.Ka;
	vec3 Id = vec3(0.0f);
	vec3 Is = vec3(0.0f);
	float dotLN = max(dot(l, n), 0.0f);

	if(dotLN > 0.0f)
	{
		Id = uLight.Ld * uMaterial.Kd * dotLN;
	    Is = uLight.Ls * uMaterial.Ks * pow(max(dot(n, h), 0.0f), uMaterial.shininess);
	}

	// intensity of reflected light
	fColor = Ia + Id + Is;

	// reflection
	vec3 reflectEnvMap = reflect(-v, n);

	// modulate with environment map reflection
	fColor *= texture(uEnvironmentMap, reflectEnvMap).rgb;
}