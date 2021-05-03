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

#pragma once
#include "GL/glew.h"
#include <memory>
#include <functional>

class OpenGLTexture
{
public:
	OpenGLTexture();
	OpenGLTexture(const unsigned char *rgba, size_t bytesPerRow, GLsizei width, GLsizei height);
	OpenGLTexture(GLuint name, GLsizei width, GLsizei height, bool flipped, std::function<void()> releaseCallback);
	~OpenGLTexture();

	GLuint	getName() const;
	GLsizei getWidth() const;
	GLsizei getHeight() const;
	bool	getFlipped() const;
	bool	isValid() const;
private:
	std::shared_ptr<GLuint> myName;
	GLsizei		myWidth;
	GLsizei		myHeight;
	bool		myFlipped;
};

