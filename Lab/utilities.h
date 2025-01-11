#ifndef UTILITIES_H
#define UTILITIES_H

// include C++ headers
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
//using namespace std;	// to avoid having to use std::

// include OpenGL related headers
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <AntTweakBar.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
//using namespace glm;	// to avoid having to use glm::

#include "ShaderProgram.h"

// vertex attribute format
struct VertexColor
{
	GLfloat position[3];
	GLfloat color[3];
};

struct VertexNormal
{
	GLfloat position[3];
	GLfloat normal[3];
};

struct VertexNormTex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texCoord[2];
};

struct VertexNormTex2
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texCoord1[2];
	GLfloat texCoord2[2];
};

struct VertexNormTanTex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat tangent[3];
	GLfloat texCoord[2];
};

// light properties
struct Light
{
	glm::vec3 pos;		// (point light/spotlight)
	glm::vec3 dir;		// (directional light/spotlight)
	glm::vec3 La;		// ambient light
	glm::vec3 Ld;		// diffuse light
	glm::vec3 Ls;		// specular light
	glm::vec3 att;		// attenuation: constant, linear, quadratic (point light/spotlight)
	float innerAngle;	// spotlight: inner angle
	float outerAngle;	// spotlight: outer angle
	int type;			// light source: 0=off; 1=point; 2=directional; 3=spotlight

	// set shader uniform variables based on type of light source
	void setLightUniforms(ShaderProgram& shader, std::string prefix, bool on = true)
	{
		std::string uniformName = prefix + "type";

		if (!on)
		{
			shader.setUniform(uniformName.c_str(), 0);
		}
		else
		{
			shader.setUniform(uniformName.c_str(), type);

			uniformName = prefix + "La";
			shader.setUniform(uniformName.c_str(), La);

			uniformName = prefix + "Ld";
			shader.setUniform(uniformName.c_str(), Ld);

			uniformName = prefix + "Ls";
			shader.setUniform(uniformName.c_str(), Ls);

			// point light
			if (type == 1)
			{
				uniformName = prefix + "pos";
				shader.setUniform(uniformName.c_str(), pos);

				uniformName = prefix + "att";
				shader.setUniform(uniformName.c_str(), att);
			}
			// directional light
			else if (type == 2)
			{
				uniformName = prefix + "dir";
				shader.setUniform(uniformName.c_str(), dir);
			}
			// spotlight
			else if (type == 3)
			{
				uniformName = prefix + "pos";
				shader.setUniform(uniformName.c_str(), pos);

				uniformName = prefix + "dir";
				shader.setUniform(uniformName.c_str(), dir);

				uniformName = prefix + "att";
				shader.setUniform(uniformName.c_str(), att);

				uniformName = prefix + "innerAngle";
				shader.setUniform(uniformName.c_str(), glm::radians(innerAngle));

				uniformName = prefix + "outerAngle";
				shader.setUniform(uniformName.c_str(), glm::radians(outerAngle));
			}
		}
	}
};

// material properties
struct Material
{
	glm::vec3 Ka;		// ambient reflection coefficient
	glm::vec3 Kd;		// diffuse reflection coefficient
	glm::vec3 Ks;		// specular reflection coefficient
	glm::vec3 emission;	// light source emission component (point light/spotlight)
	float shininess;	// specular reflection shininess exponent
};


#endif