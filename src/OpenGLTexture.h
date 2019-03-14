#pragma once
#include "GL/glew.h"
#include <memory>
#include <functional>

class OpenGLTexture
{
public:
	OpenGLTexture();
	OpenGLTexture(const unsigned char *rgba, size_t bytesPerRow, GLsizei width, GLsizei height);
    OpenGLTexture(GLuint name, GLsizei width, GLsizei height, std::function<void()> releaseCallback);
	~OpenGLTexture();
	GLuint getName() const;
	GLsizei getWidth() const;
	GLsizei getHeight() const;
    bool isValid() const;
private:
	std::shared_ptr<GLuint> myName;
	GLsizei myWidth;
	GLsizei myHeight;
};

