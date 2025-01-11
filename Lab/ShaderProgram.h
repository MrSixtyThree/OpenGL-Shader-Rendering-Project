#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <GLEW/glew.h>
#include <glm/glm.hpp>

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	// compile and link a vertex and fragment shader pair
	void compileAndLink(const std::string vShaderFilename, const std::string fShaderFilename);
	// use the shader program
	void use();

	// functions to set shader uniform variables
	void setUniform(const char *name, const glm::vec2& vector);
	void setUniform(const char *name, const glm::vec3& vector);
	void setUniform(const char *name, const glm::vec4& vector);
	void setUniform(const char* name, const glm::mat3& matrix);
	void setUniform(const char *name, const glm::mat4& matrix);
	void setUniform(const char *name, float value);
	void setUniform(const char *name, int value);
	void setUniform(const char *name, bool value);

private:
	GLuint mProgramID = 0;							// shader program handle
	std::map<std::string, GLint> mUniformLocations;	// uniform locations

	GLint getUniformLocation(const char *name);		// get uniform variable locations
};

#endif
