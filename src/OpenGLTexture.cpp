/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "stdafx.h"
#include "OpenGLTexture.h"
#include <TouchEngine/TEOpenGL.h>

OpenGLTexture::OpenGLTexture()
	: myWidth(0), myHeight(0), myFlipped(false)
{
}

OpenGLTexture::OpenGLTexture(const unsigned char * rgba, size_t bytesPerRow, GLsizei width, GLsizei height)
	: myWidth(width), myHeight(height), myFlipped(false)
{
	if (bytesPerRow != width * 4LL)
	{
		throw "Needs work";
	}
	GLuint name;
	glGenTextures(1, &name);
	glBindTexture(GL_TEXTURE_2D, name);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, rgba);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	myName = std::shared_ptr<GLuint>(new GLuint(name), [](GLuint* p) {
		glDeleteTextures(1, p);
		delete p;
	});
}

OpenGLTexture::OpenGLTexture(const TouchObject<TEOpenGLTexture> &source)
	: mySource(source), myWidth(TEOpenGLTextureGetWidth(source)), myHeight(TEOpenGLTextureGetHeight(source)), myFlipped(TETextureGetOrigin(source) != TETextureOriginBottomLeft)
{
	myName = std::make_shared<GLuint>(TEOpenGLTextureGetName(source));
}

GLuint
OpenGLTexture::getName() const
{
	return *myName;
}

GLsizei
OpenGLTexture::getWidth() const
{
	return myWidth;
}

GLsizei
OpenGLTexture::getHeight() const
{
	return myHeight;
}

bool
OpenGLTexture::getFlipped() const
{
	return myFlipped;
}

bool
OpenGLTexture::isValid() const
{
	if (myName)
	{
		return true;
	}
	return false;
}
