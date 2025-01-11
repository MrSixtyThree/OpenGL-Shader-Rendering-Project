#version 330 core

// input data
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

// uniform input data
uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelMatrix;
uniform mat3 uNormalMatrix;

// output data
out vec3 vPosition;
out vec3 vNormal;

void main()
{
	// set vertex position
    gl_Position = uModelViewProjectionMatrix * vec4(aPosition, 1.0f);

	// set vertex shader output
	// will be interpolated for each fragment
	vPosition = (uModelMatrix * vec4(aPosition, 1.0f)).xyz;
	vNormal = uNormalMatrix * aNormal;
}
