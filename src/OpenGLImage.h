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

#include "Drawable.h"
#include "OpenGLTexture.h"
#include "GL/glew.h"

class OpenGLImage : public Drawable
{
public:
	OpenGLImage();
	OpenGLImage(const OpenGLImage& o) = delete;
	OpenGLImage(OpenGLImage&& o);
	OpenGLImage& operator=(const OpenGLImage& o) = delete;
	OpenGLImage& operator=(OpenGLImage&& o);
	~OpenGLImage();

	bool	setup(GLint vertexAttribLocation, GLint textureAttribLocation);
	void	position(float x, float y);
	void	scale(float scaleX, float scaleY);
	void	draw();
	void	update(const OpenGLTexture &texture);

	const OpenGLTexture&
	getTexture() const
	{
		return myTexture;
	}
private:
	OpenGLTexture	myTexture;
	GLuint			myVAO = 0;
	GLuint			myVBO = 0;
	float			myScaleX = 1.0f;
	float			myScaleY = 1.0f;
	bool			myDirty = true;
};

