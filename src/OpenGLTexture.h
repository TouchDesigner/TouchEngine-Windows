#pragma once
#include "GL/glew.h"

class OpenGLTexture
{
public:
	OpenGLTexture();
	OpenGLTexture(const unsigned char *rgba, size_t bytesPerRow, GLsizei width, GLsizei height);
	~OpenGLTexture();
	GLuint getName() const;
	GLsizei getWidth() const;
	GLsizei getHeight() const;
private:
	GLuint myName = 0;
	GLsizei myWidth;
	GLsizei myHeight;
};

