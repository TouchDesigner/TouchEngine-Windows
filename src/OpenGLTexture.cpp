/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 *
 * TouchEngine
 *
 * Copyright © 2021 Derivative. All rights reserved.
 *
 */

#include "stdafx.h"
#include "OpenGLTexture.h"

OpenGLTexture::OpenGLTexture()
{
}

OpenGLTexture::OpenGLTexture(const unsigned char * rgba, size_t bytesPerRow, GLsizei width, GLsizei height)
	: myWidth(width), myHeight(height), myFlipped(false)
{
	if (bytesPerRow != width * 4)
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

OpenGLTexture::OpenGLTexture(GLuint name, GLsizei width, GLsizei height, bool flipped, std::function<void()> releaseCallback)
	: myWidth(width), myHeight(height), myFlipped(flipped)
{
	myName = std::shared_ptr<GLuint>(new GLuint(name), [releaseCallback](GLuint* p) {
		if (releaseCallback)
		{
			releaseCallback();
		}
		delete p;
	});
}


OpenGLTexture::~OpenGLTexture()
{
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
