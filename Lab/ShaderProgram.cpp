#include "ShaderProgram.h"

ShaderProgram::ShaderProgram() : mProgramID(0)
{}

ShaderProgram::~ShaderProgram()
{
	// check if shader program exists
	if (mProgramID != 0)
	{
		// delete the shader program
		glDeleteProgram(mProgramID);
	}
}

// compile and link a vertex and fragment shader pair
void ShaderProgram::compileAndLink(const std::string vShaderFilename, const std::string fShaderFilename)
{
	GLint status;	// used for checking compile and link status

/****************************************************************
 * Step 1: read vertex and fragment shader source code from files
 ****************************************************************/
	std::string vShaderString;	// to store vertex shader code
	std::string fShaderString;	// to store fragment shader code
	std::ifstream vShaderFile(vShaderFilename, std::ios::in); 	// open file
	std::ifstream fShaderFile(fShaderFilename, std::ios::in); 	// open file

	// if file successfully opened, get the shader source code
	if (vShaderFile.is_open())
	{
		std::stringstream stream;
		stream << vShaderFile.rdbuf();	// read buffer contents
		vShaderString = stream.str();	// convert stream into string
		vShaderFile.close();			// close file
	}
	else
	{
		// output error message and exit
		std::cerr << "Failed to open: " << vShaderFilename << std::endl;
		exit(EXIT_FAILURE);
	}

	// if file successfully opened, get the shader source code
	if (fShaderFile.is_open())
	{
		std::stringstream stream;
		stream << fShaderFile.rdbuf();	// read buffer contents
		fShaderString = stream.str();	// convert stream into string
		fShaderFile.close();			// close file
	}
	else
	{
		// output error message and exit
		std::cerr << "Failed to open: " << fShaderFilename << std::endl;
		exit(EXIT_FAILURE);
	}

/****************************************************************
 * Step 2: Create and compile shader objects
 ****************************************************************/
	 // create shader objects
	GLuint vShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// provide source code for shaders
	const GLchar *vShaderCode = vShaderString.c_str();
	const GLchar *fShaderCode = fShaderString.c_str();
	glShaderSource(vShaderID, 1, &vShaderCode, nullptr);
	glShaderSource(fShaderID, 1, &fShaderCode, nullptr);

	// compile shaders
	glCompileShader(vShaderID);
	glCompileShader(fShaderID);

	// check vertex shader compile status
	status = GL_FALSE;
	glGetShaderiv(vShaderID, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		// output error message
		std::cerr << "Failed to compile " << vShaderFilename << std::endl;

		// output error log
		int infoLogLength;
		glGetShaderiv(vShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::string errorMessage(infoLogLength, ' ');
		glGetShaderInfoLog(vShaderID, infoLogLength, nullptr, &errorMessage[0]);
		std::cerr << errorMessage << std::endl;

		exit(EXIT_FAILURE);
	}

	// check fragment shader compile status
	status = GL_FALSE;
	glGetShaderiv(fShaderID, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		// output error message
		std::cerr << "Failed to compile " << fShaderFilename << std::endl;

		// output error log
		int infoLogLength;
		glGetShaderiv(fShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::string errorMessage(infoLogLength, ' ');
		glGetShaderInfoLog(fShaderID, infoLogLength, nullptr, &errorMessage[0]);
		std::cerr << errorMessage << std::endl;

		exit(EXIT_FAILURE);
	}

/****************************************************************
 * Step 3: Attach shaders to program object and link
 ****************************************************************/
	// create program object
	mProgramID = glCreateProgram();

	// attach shaders to the program object
	glAttachShader(mProgramID, vShaderID);
	glAttachShader(mProgramID, fShaderID);

	// link program object
	glLinkProgram(mProgramID);

	// check link status
	status = GL_FALSE;
	glGetProgramiv(mProgramID, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		// output error message
		std::cerr << "Failed to link shader program." << std::endl;

		// output error log
		int infoLogLength;
		glGetShaderiv(mProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);
		std::string errorMessage(infoLogLength, ' ');
		glGetShaderInfoLog(mProgramID, infoLogLength, nullptr, &errorMessage[0]);
		std::cerr << errorMessage << std::endl;

		exit(EXIT_FAILURE);
	}

	// flag shaders for deletion (will not actually be deleted until detached from program)
	glDeleteShader(vShaderID);
	glDeleteShader(fShaderID);
}

// use the shader program
void ShaderProgram::use()
{
	// use the shader program
	glUseProgram(mProgramID);
}

void ShaderProgram::setUniform(const char *name, const glm::vec2& vector)
{
	glUniform2fv(getUniformLocation(name), 1, &vector[0]);
}

void ShaderProgram::setUniform(const char *name, const glm::vec3& vector)
{
	glUniform3fv(getUniformLocation(name), 1, &vector[0]);
}

void ShaderProgram::setUniform(const char *name, const glm::vec4& vector)
{
	glUniform4fv(getUniformLocation(name), 1, &vector[0]);
}

void ShaderProgram::setUniform(const char* name, const glm::mat3& matrix)
{
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void ShaderProgram::setUniform(const char *name, const glm::mat4& matrix)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void ShaderProgram::setUniform(const char *name, float value)
{
	glUniform1f(getUniformLocation(name), value);
}

void ShaderProgram::setUniform(const char *name, int value)
{
	glUniform1i(getUniformLocation(name), value);
}

void ShaderProgram::setUniform(const char *name, bool value)
{
	glUniform1i(getUniformLocation(name), value);
}

// get uniform variable locations
GLint ShaderProgram::getUniformLocation(const char *name)
{
	// find whether location already stored
	auto position = mUniformLocations.find(name);

	// if not stored
	if (position == mUniformLocations.end()) {
		// find the uniform's location
		GLint location = glGetUniformLocation(mProgramID, name);

		// store location in map
		mUniformLocations[name] = location;
		return location;
	}

	// return uniform's location
	return position->second;
}