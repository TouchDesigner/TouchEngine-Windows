#include "stdafx.h"
#include "OpenGLTexture.h"


OpenGLTexture::OpenGLTexture()
{
}

OpenGLTexture::OpenGLTexture(const unsigned char * rgba, size_t bytesPerRow, GLsizei width, GLsizei height)
	: myWidth(width), myHeight(height)
{
	if (bytesPerRow != width * 4)
	{
		throw "Needs work";
	}
	glGenTextures(1, &myName);
	glBindTexture(GL_TEXTURE_2D, myName);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}


OpenGLTexture::~OpenGLTexture()
{
}

GLuint OpenGLTexture::getName() const
{
	return myName;
}

GLsizei OpenGLTexture::getWidth() const
{
	return myWidth;
}

GLsizei OpenGLTexture::getHeight() const
{
	return myHeight;
}
