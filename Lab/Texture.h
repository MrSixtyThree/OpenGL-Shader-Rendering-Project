#ifndef TEXTURE_H
#define TEXTURE_H

#include "utilities.h"

class Texture
{
public:
	Texture();
	~Texture();

	// binds the texture for use
	void bind();
	// set texture parameters
	void setFilterParams(GLuint magFilter, GLuint minFilter);
	void setWrapParams(GLuint wrapS, GLuint wrapT);
	// generate a 2D texture from image data
	void generate(unsigned char* imageData, int width, int height);	
	// generate a 2D texture from an image file
	void generate(const std::string filename);
	// generate a cube environment map from image files
	void generate(const std::string fileFront, const std::string fileBack,
		const std::string fileLeft, const std::string fileRight,
		const std::string fileTop, const std::string fileBottom);

private:
	// texture ID and parameters
	GLuint mTextureID = 0;
	GLenum mTarget = 0;
	GLuint mMagFilter = GL_LINEAR;
	GLuint mMinFilter = GL_LINEAR_MIPMAP_LINEAR;
	GLuint mWrapS = GL_REPEAT;
	GLuint mWrapT = GL_REPEAT;
};

#endif

