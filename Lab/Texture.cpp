#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION   
#include "stb_image.h"

Texture::Texture()
{
	stbi_set_flip_vertically_on_load(true); // flip image about y-axis
}

Texture::~Texture()
{
	// if texture exists
	if (mTextureID != 0)
	{
		// delete texture
		glDeleteTextures(1, &mTextureID);
		mTextureID = 0;
	}
}

void Texture::bind()
{
	// if texture exists
	if (mTextureID != 0)
	{
		glBindTexture(mTarget, mTextureID);
	}
}

void Texture::setFilterParams(GLuint magFilter, GLuint minFilter)
{
	// parameter settings
	GLuint mMagFilter = magFilter;
	GLuint mMinFilter = minFilter;

	// change filters if texture exists
	if (mTextureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, mTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mMagFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mMinFilter);
	}
}

void Texture::setWrapParams(GLuint wrapS, GLuint wrapT)
{
	// parameter settings
	GLuint mWrapS = wrapS;
	GLuint mWrapT = wrapT;

	// change wrap mode if texture exists
	if (mTextureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, mTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapT);
	}
}

// generate a 2D texture from image data
void Texture::generate(unsigned char* imageData, int width, int height)
{
	// generate texture
	glGenTextures(1, &mTextureID);
	glBindTexture(GL_TEXTURE_2D, mTextureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);

	// set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mMagFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mMinFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapT);

	// set texture target
	mTarget = GL_TEXTURE_2D;
}

// generate a 2D texture from an image file
void Texture::generate(const std::string filename)
{
	// load image data
	int width, height, channels;
	unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &channels, 0);

	// if successfully loaded image
	if (imageData)
	{
		// generate texture
		glGenTextures(1, &mTextureID);
		glBindTexture(GL_TEXTURE_2D, mTextureID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mMagFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mMinFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapT);

		// free image data
		stbi_image_free(imageData);

		// set texture target
		mTarget = GL_TEXTURE_2D;
	}
	else
	{
		std::cout << "Unable to load: " << filename << std::endl;
	}
}

void Texture::generate(const std::string fileFront, const std::string fileBack,
	const std::string fileLeft, const std::string fileRight,
	const std::string fileTop, const std::string fileBottom)
{
	// load image data
	// assume images are the same size
	int width, height, channels;
	unsigned char* imageFront = stbi_load(fileFront.c_str(), &width, &height, &channels, 0);
	unsigned char* imageBack = stbi_load(fileBack.c_str(), &width, &height, &channels, 0);
	unsigned char* imageLeft = stbi_load(fileLeft.c_str(), &width, &height, &channels, 0);
	unsigned char* imageRight = stbi_load(fileRight.c_str(), &width, &height, &channels, 0);
	unsigned char* imageTop = stbi_load(fileTop.c_str(), &width, &height, &channels, 0);
	unsigned char* imageBottom = stbi_load(fileBottom.c_str(), &width, &height, &channels, 0);

	// if successfully loaded cubemap images
	if (imageFront && imageBack && imageLeft && imageRight && imageTop && imageBottom)
	{
		// generate texture
		glGenTextures(1, &mTextureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageRight);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageLeft);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageTop);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageBottom);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageBack);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageFront);

		// set texture parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// free image data
		stbi_image_free(imageFront);
		stbi_image_free(imageBack);
		stbi_image_free(imageLeft);
		stbi_image_free(imageRight);
		stbi_image_free(imageTop);
		stbi_image_free(imageBottom);

		// set texture target
		mTarget = GL_TEXTURE_CUBE_MAP;
	}
	else
	{
		std::cout << "Unable to load cubemap images starting with: " << fileFront << std::endl;
	}
}
