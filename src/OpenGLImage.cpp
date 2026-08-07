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
#include "OpenGLImage.h"

OpenGLImage::OpenGLImage()
{
}

OpenGLImage::OpenGLImage(OpenGLImage&& o) noexcept
	: Drawable(std::move(o)), myTexture(std::move(o.myTexture)), myVAO(o.myVAO), myVBO(o.myVBO)
{
	o.myVAO = 0;
	o.myVBO = 0;
}

OpenGLImage&
OpenGLImage::operator=(OpenGLImage&& o) noexcept
{
	myTexture = std::move(o.myTexture);
	std::swap(myVAO, o.myVAO);
	std::swap(myVBO, o.myVBO);
	Drawable::operator=(std::move(o));
	return *this;
}

OpenGLImage::~OpenGLImage()
{
	if (myVAO)
	{
		glDeleteVertexArrays(1, &myVAO);
	}
	if (myVBO)
	{
		glDeleteBuffers(1, &myVBO);
	}
}

bool OpenGLImage::isValid() const
{
	return myTexture.isValid();
}

bool
OpenGLImage::setup(GLint vertexAttribLocation, GLint textureAttribLocation)
{
	glGenVertexArrays(1, &myVAO);
	glGenBuffers(1, &myVBO);

	glBindVertexArray(myVAO);
	glBindBuffer(GL_ARRAY_BUFFER, myVBO);

	glEnableVertexAttribArray(vertexAttribLocation);
	glVertexAttribPointer(vertexAttribLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

	glEnableVertexAttribArray(textureAttribLocation);
	glVertexAttribPointer(textureAttribLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(2 * sizeof(GLfloat)));

	return true;
}

void
OpenGLImage::draw()
{
	if (changed)
	{
		glBindBuffer(GL_ARRAY_BUFFER, myVBO);

		GLfloat vertices[] = {
			(-1.0f * scaleX) + x,   (-1.0f * scaleY) + y,   0.0f,    myTexture.getFlipped() ? 1.0f : 0.0f,
			(-1.0f * scaleX) + x,    (1.0f * scaleY) + y,   0.0f,    myTexture.getFlipped() ? 0.0f : 1.0f,
			(1.0f * scaleX) + x,   (-1.0f * scaleY) + y,   1.0f,    myTexture.getFlipped() ? 1.0f : 0.0f,
			(1.0f * scaleX) + x,    (1.0f * scaleY) + y,   1.0f,    myTexture.getFlipped() ? 0.0f : 1.0f
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		changed = false;
	}

	if (myTexture.isValid())
	{
		glBindTexture(GL_TEXTURE_2D, myTexture.getName());
		glBindVertexArray(myVAO);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void
OpenGLImage::update(const OpenGLTexture & texture)
{
	if (myTexture.getWidth() != texture.getWidth() || myTexture.getHeight() != texture.getHeight() || myTexture.getFlipped() != texture.getFlipped())
	{
		width = float(texture.getWidth());
		height = float(texture.getHeight());
		changed = true;
	}
	myTexture = texture;
}
